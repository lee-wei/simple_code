SERVER_DIR = $(PWD)/server
IPC_CLIENT_DIR = $(PWD)/ipc_client
PC_CLIENT_DIR = $(PWD)/pc_client

ALL:
	make -C $(SERVER_DIR)
	make -C $(IPC_CLIENT_DIR)
	make -C $(PC_CLIENT_DIR)

clean:
	make -C $(SERVER_DIR) clean
	make -C $(IPC_CLIENT_DIR) clean
	make -C $(PC_CLIENT_DIR) clean
