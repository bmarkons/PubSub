#pragma once

void publishing_loop(SOCKET* socket);
void publish(ByteArray message, ByteArray topic, SOCKET* socket);
ByteArray make_package(ByteArray message, ByteArray topic);