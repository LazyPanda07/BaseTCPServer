import random
import string
import unittest
import os

from socket import *
from pathlib import Path


class EchoServerTest(unittest.TestCase):
    def test_echo(self):
        while True:
            if os.path.exists("run.txt"):
                break

        for i in range(8192):
            with create_connection(("127.0.0.1", 8080), 5) as client_socket:
                message = EchoServerTest._generate_random_message()

                client_socket.send(len(message).to_bytes(4, "little"))

                client_socket.send(bytes(message, "UTF-8"))

                length = client_socket.recv(4)

                length = int.from_bytes(length, "little")

                echo_message = client_socket.recv(length)

                self.assertEqual(message + " from echo server", echo_message.decode("UTF-8"))

    @staticmethod
    def _generate_random_message() -> str:
        return ''.join(random.choices(string.ascii_uppercase + string.digits, k=128))


if __name__ == '__main__':
    unittest.main(exit=False)

    Path("finish.txt").touch()
