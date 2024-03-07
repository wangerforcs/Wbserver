from socket import socket, AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR

# HTTPResponse ='HTTP/1.1 200 OK\r\n\r\n<html>Hello World!</html>'
HTTPResponse = 'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n'

WebSocket = socket(AF_INET, SOCK_STREAM)
WebSocket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
WebSocket.bind(('localhost', 12345))
WebSocket.listen(1)

while True:
  print('Waiting HTTP Request...')
  HTTPSocket, addr = WebSocket.accept()  # Wait Connection
  Request = HTTPSocket.recv(1024)
  
  print('Send HTTP Response!')
  HTTPSocket.send(HTTPResponse.encode('utf-8'))  # Send 
  
  HTTPSocket.send('5\r\nHello\r\n'.encode('utf-8'))
  HTTPSocket.send('0\r\n\r\n'.encode('utf-8'))

#   HTTPSocket.close()  # Close Connection