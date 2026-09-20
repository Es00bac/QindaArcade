"""Start an isolated Blender MCP session without changing Blender preferences."""
import importlib.util
import bpy

addon_path = '/home/cabewse/work_SPaC3/D3scent/tools/blender_mcp/blender_mcp.py'
spec = importlib.util.spec_from_file_location('qinda_blender_mcp', addon_path)
addon = importlib.util.module_from_spec(spec)
spec.loader.exec_module(addon)
addon.register()
server = addon.BlenderMCPServer(host='127.0.0.1', port=9877)
server.start()
bpy.types.qinda_asset_server = server
print('QINDA_ASSET_MCP_READY 9877', flush=True)
