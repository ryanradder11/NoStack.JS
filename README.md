#  NoStack.js

> An HTTP server with no frameworks, no libraries, no excuses. 

---

##  What is this?

**NoStack.js** is a minimalist HTTP server written in C++ (despite the JS in the name, because why not). No Node, no Express, no third-party anything. 

It's a small love letter to building things to understand them.

---

## Features

-  Accepts real TCP connections
-  Manually parses HTTP/1.1 requests (yes, with `\r\n`)
-  Crafts valid HTTP responses with your bare hands
-  No routers
-  No middleware
-  No nothing

---

## Getting Started

```bash
g++ -o nostack main.cpp

```

```bash
./nostack

```

Then 
```bash
curl http://localhost:8080
```