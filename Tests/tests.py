import random
import string
import unittest

from socket import *


class EchoServerTest(unittest.TestCase):
    def test_echo(self):
        for i in range(8192):
            with create_connection(("127.0.0.1", 80), 5) as socket:
                message = EchoServerTest._generate_random_message()

                socket.send(len(message).to_bytes(4, "little"))

                socket.send(bytes(message, "UTF-8"))

                length = socket.recv(4)

                length = int.from_bytes(length, "little")

                echo_message = socket.recv(length)

                self.assertEqual(message + " from echo server", echo_message.decode("UTF-8"))

    @staticmethod
    def _generate_random_message() -> str:
        return ''.join(random.choices(string.ascii_uppercase + string.digits, k=128))


if __name__ == '__main__':
    unittest.main()
