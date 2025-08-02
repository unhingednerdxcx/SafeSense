import socket

HOST = '0.0.0.0'  # Accept from any IP
PORT = 12245

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print(f"[LISTENING] Server is listening on port {PORT}")

class codes:
    # Codes for future use
    ACCEPT = "111"
    REJECT = "000"

def collect_data():
    while True:
        client_socket, addr = server.accept()
        print(f"[CONNECTED] ESP at {addr}")

        try:
            # Step 1: receive "code : rq : 120"
            request = client_socket.recv(1024).decode().strip()
            print(f"[ESP] Request: {request}")

            # Step 2: check if it ends with :: : rq : 120 (Pair request)
            if request.endswith(" : rq : 120"):
                client_socket.sendall(b"111\n")  # Send Accept
                print("[SERVER] Sent Accept (111)")

                # Step 3: receive the data
                data = b""
                client_socket.settimeout(2.0)  # allow time for ESP to send
                try:
                    while True:
                        chunk = client_socket.recv(1024)
                        if not chunk:
                            break
                        data += chunk
                except socket.timeout:
                    pass

                data = data.decode().strip()
                print(f"[ESP] Data: {repr(data)}")

                print(f"[ESP] Data: {data}")

                parts = data.split(" : ")
                if len(parts) >= 8:
                    print(f"[PARSED] temp={parts[2]}  volume={parts[3]}  dist={parts[5]}  ppm={parts[6]}")
                    data_Array = [parts[2], parts[3], parts[5], parts[6]]
                    return data_Array
            else:
                print("[SERVER] Unexpected message. Ignored.")

        except Exception as e:
            print(f"[ERROR] {e}")
        finally:
            client_socket.close()
