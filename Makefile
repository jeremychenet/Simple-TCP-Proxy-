PATH_SERVER = ./server
PATH_CLIENT = ./client

all :
	make -C $(PATH_SERVER)
	make -C $(PATH_CLIENT)

clean:
	make clean -C $(PATH_SERVER)
	make clean -C $(PATH_CLIENT)

fclean:	clean
	make fclean -C $(PATH_SERVER)
	make fclean -C $(PATH_CLIENT)

re: fclean all