# SPDX-FileCopyrightText: 2024 Hardronix
#
# SPDX-License-Identifier: GPL-2.0-or-later

import bpy

bl_info = {
    "name": "Nx::Blender",
    "author": "Hardronix",
    "version": (0, 0, 1),
    "blender": (4, 2, 0),
    "location": "Global",
    "description": "NX features support",
    "warning": "",
    "doc_url": "",
    "support": 'OFFICIAL',
    "category": "Global",
}

def register() -> None:
    print("NX init!")

def unregister() -> None:
    print("NX uninit!")