SRC			=	main.cpp 	\
				client.cpp 	\

CXX 		= 	c++
CXXFLAGS	=	-std=c++20 

ERRORFLAGS	=	-Wall -Wextra -Werror
LIBFLAGS 	=	-lboost_system -lpthread

OBJ			=	$(SRC:.cpp=.o)
NAME		=	client_side

all :	$(OBJ)
	$(CXX) -o $(NAME) $(OBJ) $(CXXFLAGS) $(LIBFLAGS) $(ERRORFLAGS)
	cp $(NAME) ../

clean:
	rm -f $(OBJ)

fclean:	clean
	rm -f $(NAME)

re: fclean all