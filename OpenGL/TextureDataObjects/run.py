"""
Script for starting a server with header files
using Python's server module. This is based on the
following reference:

https://stackoverflow.com/a/13354482
by berto (https://stackoverflow.com/users/703144/berto)
ed. by Flimm (https://stackoverflow.com/users/247696/flimm)
original question by nynexman4464
(https://stackoverflow.com/users/373446/nynexman4464)

"""

from http import server
import sys


class RequestHandler(server.SimpleHTTPRequestHandler):

    def end_headers(self):
        self.send_headers()
        server.SimpleHTTPRequestHandler.end_headers(self)

    def send_headers(self):
        self.send_header("Cross-Origin-Embedder-Policy",
                         "require-corp")
        self.send_header("Cross-Origin-Opener-Policy",
                         "same-origin")

if __name__ == '__main__':
    port = 8000
    if len(sys.argv) > 1:
        port = sys.argv[1] 
    server.test(HandlerClass=RequestHandler, port=port)
