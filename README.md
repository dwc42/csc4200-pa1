
# Compilation Instructions
1. cd into a sprint folder
2. On Client: 
```gcc -fsanitize=address -g socket_client.c socket_utilities.c -o client.out -lm```
   On Server: 
```gcc -fsanitize=address -g socket_server.c socket_utilities.c -o server.out -lm```
   sprint1 Client:
```gcc -fsanitize=address -g socket_client.c -o client.out -lm```
   sprint1 Server: 
```gcc -fsanitize=address -g socket_server.c -o server.out -lm```
3. On Server First: 
   ```./server.out```
   Then Client: 
   ```./client.out```
   


