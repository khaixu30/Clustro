To run this file just run this command in terminal.

```bash
g++ src/main.cpp src/includes/clustro.cpp src/includes/user.cpp src/includes/post.cpp src/includes/storage.cpp src/includes/notification.cpp src/includes/feedmanager.cpp src/includes/comment.cpp src/includes/followmanager.cpp -o build/clustro_app -std=c++17 -lstdc++fs -I src/includes
```

then run
```bash
./build/clustro_app.exe
```