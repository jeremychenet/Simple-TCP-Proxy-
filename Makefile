PATH_SERVER = ./server
PATH_CLIENT = ./client
SERVER_BIN = server_side
CLIENT_BIN = client_side

all :
	make -C $(PATH_SERVER)
	make -C $(PATH_CLIENT)

clean:
	make clean -C $(PATH_SERVER)
	make clean -C $(PATH_CLIENT)

fclean:	clean
	make fclean -C $(PATH_SERVER)
	make fclean -C $(PATH_CLIENT)
	rm -f $(SERVER_BIN)
	rm -f $(CLIENT_BIN)
	rm -f log.txt

re: fclean all