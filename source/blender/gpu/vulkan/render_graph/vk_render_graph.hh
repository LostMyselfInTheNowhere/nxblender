/* SPDX-FileCopyrightText: 2024 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 *
 * The render graph primarily is a a graph of GPU commands that are then serialized into command
 * buffers. The submission order can be altered and barriers are added for resource sync.
 *
 * # Building render graph
 *
 * The graph contains nodes that refers to resources it reads from, or modifies.
 * The resources that are read from are linked to the node inputs. The resources that are written
 * to are linked to the node outputs.
 *
 * Resources needs to be tracked as usage can alter the content of the resource. For example an
 * image can be optimized for data transfer, or optimized for sampling which can use a different
 * pixel layout on the device.
 *
 * When adding a node to the render graph the input and output links are extracted from the
 * See `VKNodeInfo::build_links`.
 *
 * # Executing render graph
 *
 * Executing a render graph is done by calling `submit_for_read` or `submit_for_present`. When
 * called the nodes that are needed to render the resource are determined by a `VKScheduler`. The
 * nodes are converted to `vkCmd*` and recorded in the command buffer by `VKCommandBuilder`.
 *
 * # Thread safety
 *
 * When the render graph is called the device will be locked. Nodes inside the render graph relies
 * on the resources which are device specific. The locked time is tiny when adding new nodes.
 * During execution this takes a longer time, but the lock can be released when the commands have
 * been queued. So other threads can continue.
 */

#pragma once

#include <mutex>
#include <optional>

#include "BKE_global.hh"

#include "BLI_map.hh"
#include "BLI_utility_mixins.hh"
#include "BLI_vector.hh"
#include "BLI_vector_set.hh"

#include "BKE_global.hh"

#include "vk_common.hh"

#include "vk_command_buffer_wrapper.hh"
#include "vk_command_builder.hh"
#include "vk_render_graph_links.hh"
#include "vk_resource_state_tracker.hh"

namespace blender::gpu::render_graph {
class VKScheduler;

class VKRenderGraph : public NonCopyable {
  friend class VKCommandBuilder;
  friend class VKScheduler;
  using DebugGroupNameID = int64_t;
  using DebugGroupID = int64_t;

  /** All links inside the graph indexable via NodeHandle. */
  Vector<VKRenderGraphNodeLinks> links_;
  /** All nodes inside the graph indexable via NodeHandle. */
  Vector<VKRenderGraphNode> nodes_;
  /** Scheduler decides which nodes to select and in what order to execute them. */
  VKScheduler scheduler_;
  /**
   * Command builder generated the commands of the nodes and record them into the command buffer.
   */
  VKCommandBuilder command_builder_;

  /**
   * Command buffer sends the commands to the device (`VKCommandBufferWrapper`).
   *
   * To improve testability the command buffer can be replaced by an instance of
   * `VKCommandBufferLog` this way test cases don't need to create a fully working context in order
   * to test something render graph specific.
   */
  std::unique_ptr<VKCommandBufferInterface> command_buffer_;

  /**
   * Not owning pointer to device resources.
   *
   * To improve testability the render graph doesn't access VKDevice or VKBackend directly.
   * resources_ can be replaced by a local variable. This way test cases don't need to create a
   * fully working context in order to test something render graph specific. Is marked optional as
   * device could
   */
  VKResourceStateTracker &resources_;

  struct {
    VectorSet<std::string> group_names;

    /** Current stack of debug group names. */
    Vector<DebugGroupNameID> group_stack;
    /** Has a node been added to the current stack? If not the group stack will be added to
     * used_groups.*/
    bool group_used = false;
    /** All used debug groups. */
    Vector<Vector<DebugGroupNameID>> used_groups;
    /**
     * Map of a node_handle to an index of debug group in used_groups.
     *
     * <source>
     * int used_group_id = node_group_map[node_handle];
     * const Vector<DebugGroupNameID> &used_group = used_groups[used_group_id];
     * </source>
     */
    Vector<DebugGroupID> node_group_map;
  } debug_;

 public:
  /**
   * Construct a new render graph instance.
   *
   * To improve testability the command buffer and resources they work on are provided as a
   * parameter.
   */
  VKRenderGraph(std::unique_ptr<VKCommandBufferInterface> command_buffer,
                VKResourceStateTracker &resources);

  /**
   * Free all resources held by the render graph. After calling this function the render graph may
   * not work as expected, leading to crashes.
   *
   * Freeing data of context resources cannot be done inside the destructor due to an issue when
   * Blender (read window manager) exits. During this phase the backend is deallocated, device is
   * destroyed, but window manager requires a context so it creates new one. We work around this
   * issue by ensuring the VKDevice is always in control of releasing resources.
   */
  void free_data();

 private:
  /**
   * Add a node to the render graph.
   */
  template<typename NodeInfo> void add_node(const typename NodeInfo::CreateInfo &create_info)
  {
    std::scoped_lock lock(resources_.mutex);
    static VKRenderGraphNode node_template = {};
    NodeHandle node_handle = nodes_.append_and_get_index(node_template);
    if (nodes_.size() > links_.size()) {
      links_.resize(nodes_.size());
    }
    VKRenderGraphNode &node = nodes_[node_handle];
    node.set_node_data<NodeInfo>(create_info);

    VKRenderGraphNodeLinks &node_links = links_[node_handle];
    BLI_assert(node_links.inputs.is_empty());
    BLI_assert(node_links.outputs.is_empty());
    node.build_links<NodeInfo>(resources_, node_links, create_info);

    if (G.debug & G_DEBUG_GPU) {
      if (!debug_.group_used) {
        debug_.group_used = true;
        debug_.used_groups.append(debug_.group_stack);
      }
      if (nodes_.size() > debug_.node_group_map.size()) {
        debug_.node_group_map.resize(nodes_.size());
      }
      debug_.node_group_map[node_handle] = debug_.used_groups.size() - 1;
    }
  }

 public:
#define ADD_NODE(NODE_CLASS) \
  void add_node(const NODE_CLASS::CreateInfo &create_info) \
  { \
    add_node<NODE_CLASS>(create_info); \
  }
  ADD_NODE(VKBeginRenderingNode)
  ADD_NODE(VKEndRenderingNode)
  ADD_NODE(VKClearAttachmentsNode)
  ADD_NODE(VKClearColorImageNode)
  ADD_NODE(VKClearDepthStencilImageNode)
  ADD_NODE(VKFillBufferNode)
  ADD_NODE(VKCopyBufferNode)
  ADD_NODE(VKCopyBufferToImageNode)
  ADD_NODE(VKCopyImageNode)
  ADD_NODE(VKCopyImageToBufferNode)
  ADD_NODE(VKBlitImageNode)
  ADD_NODE(VKDispatchNode)
  ADD_NODE(VKDispatchIndirectNode)
  ADD_NODE(VKDrawNode)
  ADD_NODE(VKDrawIndexedNode)
  ADD_NODE(VKDrawIndexedIndirectNode)
  ADD_NODE(VKDrawIndirectNode)
  ADD_NODE(VKUpdateMipmapsNode)
#undef ADD_NODE

  /**
   * Submit partial graph to be able to read the expected result of the rendering commands
   * affecting the given vk_buffer. This method is called from
   * `GPU_texture/storagebuf/indexbuf/vertbuf/_read`. In vulkan the content of images cannot be
   * read directly and always needs to be copied to a transfer buffer.
   *
   * After calling this function the mapped memory of the vk_buffer would contain the data of the
   * buffer.
   */
  void submit_buffer_for_read(VkBuffer vk_buffer);

  /**
   * Submit partial graph to be able to present the expected result of the rendering commands
   * affecting the given vk_swapchain_image. This method is called when performing a
   * swap chain swap.
   *
   * Pre conditions:
   * - `vk_swapchain_image` needs to be a created using ResourceOwner::SWAP_CHAIN`.
   *
   * Post conditions:
   * - `vk_swapchain_image` layout is transitioned to `VK_IMAGE_LAYOUT_SRC_PRESENT`.
   */
  void submit_for_present(VkImage vk_swapchain_image);

  /**
   * Submit full graph.
   */
  void submit();

  /**
   * Push a new debugging group to the stack with the given name.
   *
   * New nodes added to the render graph will be associated with this debug group.
   */
  void debug_group_begin(const char *name);

  /**
   * Pop the top of the debugging group stack.
   *
   * New nodes added to the render graph will be associated with the parent of the current debug
   * group.
   */
  void debug_group_end();

 private:
  void remove_nodes(Span<NodeHandle> node_handles);
};

}  // namespace blender::gpu::render_graph
