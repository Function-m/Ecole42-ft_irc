# 글자 ANSI 정의
BOLD        =   \033[1m
GREEN       =   \033[0;32m
INIT_ANSI   =   \033[0m     # 초기화

# 실행 파일 이름
NAME = ircserv
DEBUG_NAME = ircserv_debug

# 소스 파일 및 오브젝트 파일 찾기 및 정의
SRC_DIR = source
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/objs
DEBUG_OBJ_DIR = $(BUILD_DIR)/debug_objs
DEP_DIR = $(BUILD_DIR)/deps
DEBUG_DEP_DIR = $(BUILD_DIR)/debug_deps
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEBUG_OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(DEBUG_OBJ_DIR)/%.o)
DEPENDS = $(OBJECTS:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)
DEBUG_DEPENDS = $(DEBUG_OBJECTS:$(DEBUG_OBJ_DIR)/%.o=$(DEBUG_DEP_DIR)/%.d)

# 컴파일러 설정
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCLUDE_DIR) -g -fsanitize=address
DEBUG_CXXFLAGS = $(CXXFLAGS) -DENABLE_DEBUG -g

# 기본 타겟 설정
.PHONY: all clean fclean re debug

# 'make all' 또는 'make' 규칙
all: $(NAME)
	@echo "\n$(GREEN)$(BOLD)Build complete.$(INIT_ANSI)"

# 실행 파일 빌드 규칙
$(NAME): $(OBJECTS)
	@echo "\n$(GREEN)Linking...$(INIT_ANSI)"
	$(CXX) $(CXXFLAGS) -o $@ $^

# 디버그 빌드 규칙
debug: $(DEBUG_NAME)
	@echo "\n$(GREEN)$(BOLD)Debug build complete.$(INIT_ANSI)"

# 디버그 실행 파일 빌드 규칙
$(DEBUG_NAME): $(DEBUG_OBJECTS)
	@echo "\n$(GREEN)Linking debug build...$(INIT_ANSI)"
	$(CXX) $(DEBUG_CXXFLAGS) -o $@ $^

# 오브젝트 파일 및 의존성 파일 빌드 규칙
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "\n$(GREEN)Compiling $< and generating dependencies...$(INIT_ANSI)"
	@mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@ -MF $(DEP_DIR)/$*.d

# 디버그 오브젝트 파일 및 의존성 파일 빌드 규칙
$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "\n$(GREEN)Compiling $< for debug and generating dependencies...$(INIT_ANSI)"
	@mkdir -p $(DEBUG_OBJ_DIR) $(DEBUG_DEP_DIR)
	$(CXX) $(DEBUG_CXXFLAGS) -MMD -MP -c $< -o $@ -MF $(DEBUG_DEP_DIR)/$*.d

# 'make clean' 규칙
clean:
	@echo "\n$(GREEN)Cleaning...$(INIT_ANSI)"
	rm -rf $(BUILD_DIR)
	@echo "\n$(GREEN)$(BOLD)Cleaned.$(INIT_ANSI)"

# 'make fclean' 규칙
fclean:
	@echo "\n$(GREEN)FCleaning...$(INIT_ANSI)"
	rm -rf $(BUILD_DIR) $(NAME) $(DEBUG_NAME)
	@echo "\n$(GREEN)$(BOLD)Cleaned.$(INIT_ANSI)"

# 'make re' 규칙
re:
	$(MAKE) clean
	$(MAKE) all

# 일반 컴파일 의존성 파일 포함
-include $(DEPENDS)

# 디버그 컴파일 의존성 파일 포함
-include $(DEBUG_DEPENDS)
