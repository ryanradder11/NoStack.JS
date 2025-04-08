# 🧱 NoStack.js

> An HTTP server with no frameworks, no libraries, no excuses. Just raw sockets and the bitter taste of RFC 2616.

---

## ⚡ What is this?

**NoStack.js** is a minimalist HTTP server written in C++ (despite the name — because why not). No Node, no Express, no third-party anything. Just **you, your terminal, and the terrifying silence of a raw TCP socket**.

It's a love letter to low-level networking and a middle finger to dependency hell.

---

## 🔥 Features

- ✅ Accepts real TCP connections
- ✅ Manually parses HTTP/1.1 requests (yes, with `\r\n`)
- ✅ Crafts valid HTTP responses with your bare hands
- ❌ No routers
- ❌ No middleware
- ❌ No mercy

---

## 💾 Getting Started

```bash
g++ -o nostack main.cpp
./nostack
```

Then 
```bash
curl http://localhost:8080
```