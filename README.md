# Simple web server on C

This is a simple HTTP server wrote on `C`. 

Default port is 8888 and ROOT for the server is your current working directory.

You can provide command line arguments like: `$./server -p [port] -r [path]`


compile - `gcc server.c -o server`

for example:
```
$./server -p 50000 -r /home/
```
to start a server at port 50000 with root directory as "/home"

```
$./server -r /home/web-server
```
to start the server at port 8888 with ROOT as /home/web-server
