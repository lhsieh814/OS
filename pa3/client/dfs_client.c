#include "client/dfs_client.h"
#include "datanode/ext.h"

int connect_to_nn(char* address, int port)
{
	assert(address != NULL);
	assert(port >= 1 && port <= 65535);
	//TODO: create a socket and connect it to the server (address, port)
	//assign return value to client_socket 
	int client_socket = create_client_tcp_socket(address, port);
	assert(client_socket != INVALID_SOCKET);

	return client_socket;
}

int modify_file(char *ip, int port, const char* filename, int file_size, int start_addr, int end_addr)
{
	int namenode_socket = connect_to_nn(ip, port);
	if (namenode_socket == INVALID_SOCKET) return -1;
	FILE* file = fopen(filename, "rb");
	assert(file != NULL);

	//TODO:fill the request and send
	dfs_cm_client_req_t request;
    memset(&request, '0', sizeof(request));
    strcpy(request.file_name, filename);
    request.req_type = 3;
    request.file_size = file_size;

	send_data(namenode_socket, &request, sizeof(request));

	//TODO: receive the response
	dfs_cm_file_res_t response;
	receive_data(namenode_socket, &response, sizeof(response));

	//TODO: send the updated block to the proper datanode
	int starting_block = start_addr/(DFS_BLOCK_SIZE - 1);
	int num_new_blocks = (end_addr - start_addr)/(DFS_BLOCK_SIZE - 1);

	int i;
	for (i = starting_block; i < starting_block + num_new_blocks; i++)
	{
		dfs_cm_block_t block = response.query_result.block_list[i];

		char *buf = (char *)malloc(sizeof(char)*DFS_BLOCK_SIZE);
		memset(buf, 0, sizeof(char)*DFS_BLOCK_SIZE);
		fseek(file, start_addr, SEEK_SET);
		fread(buf, DFS_BLOCK_SIZE, 1, file);
		strncpy(block.content, buf, DFS_BLOCK_SIZE);

		int datanode_socket = connect_to_nn(block.loc_ip, block.loc_port);

		dfs_cli_dn_req_t datanode_req;
		memset(&datanode_req, '0', sizeof(datanode_req));
		datanode_req.op_type = 1;
		datanode_req.block = block;

		send_data(datanode_socket, &datanode_req, sizeof(datanode_req));
	}

	fclose(file);
	return 0;
}

int push_file(int namenode_socket, const char* local_path)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(local_path != NULL);
	FILE *file = fopen(local_path, "rb");
	assert(file != NULL);

	// Create the push request (write)
	dfs_cm_client_req_t request;
    memset(&request, '0', sizeof(request));

	//TODO:fill the fields in request and 
	request.req_type = 1;
	strcpy(request.file_name, local_path);
	// Get the size of the file
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	request.file_size = size;
	send_data(namenode_socket, &request, sizeof(request));

	//TODO:Receive the response
	dfs_cm_file_res_t response;
	receive_data(namenode_socket, &response, sizeof(response));

	//TODO: Send blocks to datanodes one by one
	dfs_cm_file_t file_desc = response.query_result;

	int i = 0;
	for (i = 0; i < file_desc.blocknum; i++)
	{
		dfs_cm_block_t block = file_desc.block_list[i];

		char *buf = (char *)malloc(sizeof(char)*DFS_BLOCK_SIZE);
		memset(buf, 0, sizeof(char)*DFS_BLOCK_SIZE);
		fread(buf, DFS_BLOCK_SIZE, 1, file);
		strncpy(block.content, buf, DFS_BLOCK_SIZE);

		int datanode_socket = connect_to_nn(block.loc_ip, block.loc_port);

		dfs_cli_dn_req_t datanode_req;
		memset(&datanode_req, '0', sizeof(datanode_req));
		datanode_req.op_type = 1;
		datanode_req.block = block;

		send_data(datanode_socket, &datanode_req, sizeof(datanode_req));
	}

	fclose(file);
	return 0;
}

int pull_file(int namenode_socket, const char *filename)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(filename != NULL);

	//TODO: fill the request, and send (read request)
	dfs_cm_client_req_t request;
	strcpy(request.file_name, filename);
	request.req_type = 0;
	send_data(namenode_socket, &request, sizeof(request));

	//TODO: Get the response
	dfs_cm_file_res_t response;
	receive_data(namenode_socket, &response, sizeof(response));
	
	dfs_cm_file_t file_desc = response.query_result;
	char complete_file[file_desc.file_size];

	//TODO: Receive blocks from datanodes one by one
	int i = 0;
	for (i = 0; i < file_desc.blocknum; i++)
	{
		dfs_cm_block_t block = file_desc.block_list[i];
		
		int datanode_socket = connect_to_nn(block.loc_ip, block.loc_port);

		dfs_cli_dn_req_t datanode_req;
		memset(&datanode_req, '0', sizeof(datanode_req));
		datanode_req.op_type = 0; //Datanode read block to client
		datanode_req.block = block;

		send_data(datanode_socket, &datanode_req, sizeof(datanode_req));

		char buffer[DFS_BLOCK_SIZE];

		receive_data(datanode_socket, &buffer, sizeof(buffer));

		strcpy(complete_file + (i * DFS_BLOCK_SIZE), buffer);

	}
	//TODO: resemble the received blocks into the complete file
	FILE *file = fopen(filename, "wb");
	fwrite(complete_file, sizeof(char), sizeof(complete_file), file);
	fclose(file);

	return 0;
}

dfs_system_status *get_system_info(int namenode_socket)
{
	assert(namenode_socket != INVALID_SOCKET);
	//TODO fill the result and send 
	dfs_cm_client_req_t request;
    memset(&request, '0', sizeof(request));
	request.req_type = 2;

	send_data(namenode_socket, &request, sizeof(request));

	//TODO: get the response
	dfs_system_status *response = (dfs_system_status *) malloc(sizeof(dfs_system_status));
	receive_data(namenode_socket, response, sizeof(response));

	return response;		
}

int send_file_request(char **argv, char *filename, int op_type)
{
	int namenode_socket = connect_to_nn(argv[1], atoi(argv[2]));
	if (namenode_socket < 0)
	{
		return -1;
	}

	int result = 1;
	switch (op_type)
	{
		case 0:
			result = pull_file(namenode_socket, filename);
			break;
		case 1:
			result = push_file(namenode_socket, filename);
			break;
	}
	close(namenode_socket);
	return result;
}

dfs_system_status *send_sysinfo_request(char **argv)
{
	int namenode_socket = connect_to_nn(argv[1], atoi(argv[2]));
	if (namenode_socket < 0)
	{
		printf("ERROR connecting client\n");
		return NULL;
	}
	dfs_system_status* ret =  get_system_info(namenode_socket);
	close(namenode_socket);
	return ret;
}
