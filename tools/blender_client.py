"""Send local asset-authoring requests through the installed Blender MCP protocol."""
import argparse
import json
from pathlib import Path
import socket

parser = argparse.ArgumentParser()
parser.add_argument('command', choices=['ping', 'scene', 'exec'])
parser.add_argument('script', nargs='?')
parser.add_argument('--port', type=int, default=9877)
args = parser.parse_args()
request = {'type': {'ping': 'ping', 'scene': 'get_scene_info', 'exec': 'execute_code'}[args.command], 'params': {}}
if args.command == 'exec':
    request['params']['code'] = Path(args.script).read_text()
with socket.create_connection(('127.0.0.1', args.port), timeout=5) as connection:
    connection.settimeout(300)
    connection.sendall(json.dumps(request).encode())
    response = b''
    while True:
        chunk = connection.recv(65536)
        if not chunk:
            raise RuntimeError('Blender closed its connection before returning a result')
        response += chunk
        try:
            result = json.loads(response)
            break
        except json.JSONDecodeError:
            pass
print(json.dumps(result, indent=2))
raise SystemExit(0 if result.get('status') == 'success' else 1)
