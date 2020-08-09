import webbrowser
from platform import system
import os
import sys
import random
import threading
from http import server
import socketserver


browser = None
if system() == 'Windows':
    browser = 'C:/Program Files (x86)/Google/Chrome/Application/chrome.exe %s'
    # firefox = 'C:/Program Files/Mozilla Firefox/firefox.exe %s'


def copy_file_contents(dest: str, src: str, 
                       start_key: str=None, 
                       end_key: str=None) -> None:
    """
    Copy the contents of a source file and place it into
    a destination file.
    """
    start_key = 'START_OF_FILE' if start_key is None else start_key
    end_key = 'END_OF_FILE' if end_key is None else end_key
    dest_file_list: list = [[], [], []]
    BEFORE, IN, AFTER = 0, 1, 2
    start_key_in_file, end_key_in_file = False, False
    state: int = BEFORE
    with open(dest, "r") as dest_file:
        for line in dest_file:
            if start_key in line and state == BEFORE:
                start_key_in_file = True
                state = IN
            elif end_key in line and state == IN:
                end_key_in_file = True
                state = AFTER
            dest_file_list[state].append(line)
    if (not start_key_in_file) or (not end_key_in_file):
        return # Also need to consider if keys are in order.
    dest_file_list[IN] = [f'// {start_key} \n']
    if src != "":
        with open(src, "r") as src_file:
            for line in src_file:
                dest_file_list[IN].append(line)
    dest_file_list[IN].append('\n')
    with open(dest, "w") as dest_file:
        for section in dest_file_list:
            for line in section:
                dest_file.write(line)



class LaunchNewBrowserThread(threading.Thread):

    def run(self):
        webbrowser.get(browser).open(f'http://localhost:{port}')


if __name__ == "__main__":
    target_file = "./scripts/shader-sources.js"
    vertex_file = "./shaders/vertices.vert"
    fragment_file = "./shaders/fragments.frag"
    if len(sys.argv) == 1:
        copy_file_contents(target_file, vertex_file, "VS_START", "VS_END")
        copy_file_contents(target_file, fragment_file, "FS_START", "FS_END")
        path = os.path.abspath(".")
        port = int(random.random()*50000) + 1024
        thread = LaunchNewBrowserThread()
        thread.start()
        # To properly get the server to load module scripts,
        # the following question with an edited answer is consulted:
        # https://stackoverflow.com/questions/59908927/
        # quesiton and answer by Fabrizio Apuzzo 
        # (https://stackoverflow.com/users/11150350/)
        req_handler = server.SimpleHTTPRequestHandler
        req_handler.extensions_map.update({".js": "application/javascript"})
        with socketserver.TCPServer(("", port), req_handler) as simple_server:
            print(f'Serving HTTP on :: port {port} (http://[::]:{port}/) ...')
            simple_server.serve_forever()

    elif len(sys.argv) == 2:
        if sys.argv[1] == 'clear':
            copy_file_contents(target_file, "", "VS_START", "VS_END")
            copy_file_contents(target_file, "", "FS_START", "FS_END")

