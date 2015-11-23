import sys
import os
import posixpath
import urllib
import BaseHTTPServer
from SimpleHTTPServer import SimpleHTTPRequestHandler


class RequestHandler(SimpleHTTPRequestHandler):
    
    def translate_path(self, path):
        """translate path given routes"""

        print path

        root = os.getcwd() + "/"
        if len(sys.argv) > 2 :
            root += sys.argv[2]
        else :
            root += "protocol/rawmsg/public"

        if path == "/" :
            path = root
        else :
            path = root + path

        path = path.split('?',1)[0]
        path = path.split('#',1)[0]
        path = posixpath.normpath(urllib.unquote(path))
        words = path.split('/')
        words = filter(None, words)
        path = "/"
        for word in words:
            drive, word = os.path.splitdrive(word)
            head, word = os.path.split(word)
            if word in (os.curdir, os.pardir): continue
            path = os.path.join(path, word)

        print path
        return path

if __name__ == '__main__':
    BaseHTTPServer.test(RequestHandler, BaseHTTPServer.HTTPServer)

