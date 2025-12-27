#!/usr/bin/env python3
"""
Load test script for TCP database server.
Spawns multiple clients that send pipelined commands concurrently.
"""

import socket
import threading
import time
import random

SERVER_HOST = 'localhost'
SERVER_PORT = 8080
NUM_CLIENTS = 10
COMMANDS_PER_CLIENT = 100

def client_worker(client_id):
    """Each client sends multiple pipelined commands"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_HOST, SERVER_PORT))
        
        # Build pipelined commands (all at once)
        commands = []
        for i in range(COMMANDS_PER_CLIENT):
            key = f"client{client_id}_key{i}"
            value = f"value{i}"
            commands.append(f"SET {key} {value}\n")
        
        # Add some GETs
        for i in range(10):
            key = f"client{client_id}_key{random.randint(0, COMMANDS_PER_CLIENT-1)}"
            commands.append(f"GET {key}\n")
        
        # Send all commands at once (pipelining!)
        batch = "".join(commands)
        sock.sendall(batch.encode())
        
        # Receive all responses
        responses = 0
        data = b""
        while responses < len(commands):
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
            # Count RESP responses (look for \r\n terminators)
            responses = data.count(b'\r\n')
        
        sock.close()
        print(f"Client {client_id}: Sent {len(commands)} commands, got {responses} responses")
        
    except Exception as e:
        print(f"Client {client_id} error: {e}")

def main():
    print(f"Starting load test: {NUM_CLIENTS} clients × {COMMANDS_PER_CLIENT} commands each")
    print(f"Total commands: {NUM_CLIENTS * (COMMANDS_PER_CLIENT + 10)}")
    
    start_time = time.time()
    
    # Spawn all clients
    threads = []
    for i in range(NUM_CLIENTS):
        t = threading.Thread(target=client_worker, args=(i,))
        t.start()
        threads.append(t)
    
    # Wait for all to complete
    for t in threads:
        t.join()
    
    elapsed = time.time() - start_time
    total_commands = NUM_CLIENTS * (COMMANDS_PER_CLIENT + 10)
    
    print(f"\n✅ Complete!")
    print(f"Time: {elapsed:.2f}s")
    print(f"Throughput: {total_commands / elapsed:.0f} commands/sec")

if __name__ == "__main__":
    main()
