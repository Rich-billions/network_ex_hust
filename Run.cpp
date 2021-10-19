#include "Server.h"

int main() {
	Server srv;
	if (srv.WinsockStartup() == -1) return 1;
	if (srv.ServerStartup() == -1) return 1;
	if (srv.ListenStartup() == -1) return 1;
	if (srv.Loop() == -1) return 1;
	return 0;
}