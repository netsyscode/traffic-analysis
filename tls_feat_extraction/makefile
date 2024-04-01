include /usr/local/etc/PcapPlusPlus.mk
PCAPPLUSPLUS_HOME := /root/Downloads/PcapPlusPlus-22.11
SOURCES := $(wildcard Src/*.cpp)
OBJS_FILENAMES := $(patsubst Src/%.cpp,Obj/%.o,$(SOURCES))
HASH_LIB_HOME := $(PCAPPLUSPLUS_HOME)/3rdParty/hash-library
HASH_LIB_SOURCES := $(wildcard $(HASH_LIB_HOME)/*.cpp)
HASH_LIB_OBJS_FILENAMES := $(patsubst $(HASH_LIB_HOME)/%.cpp,$(HASH_LIB_HOME)/Obj/%.o,$(HASH_LIB_SOURCES))

INCLUDES := -I"./Src" \
			-I"./Inc" \
			-I"$(PCAPPP_INCLUDES)" \
			-I"$(HASH_LIB_HOME)" \
			-I"/usr/include"  # 添加MySQL Connector/C++的头文件路径

ifdef BUILD_WITH_STATIC_LIBPCAP
INCLUDES += -I"$(STATIC_LIBPCAP_INCLUDE)"
endif

Obj/%.o: Src/%.cpp
	@echo Building file: $<
	@$(CXX) $(PCAPPP_BUILD_FLAGS) -g -c $(INCLUDES) -fmessage-length=0 -MMD -MP -MF "$(@:Obj/%.o=Obj/%.d)" -MT "$(@:Obj/%.o=Obj/%.d)" -o "$@" "$<"

UNAME := $(shell uname)
CUR_TARGET := $(notdir $(shell pwd))

.SILENT:

all: dependents TLSFingerprinting

start:
	@echo ==== Building target: $(CUR_TARGET) ====

create-directories:
	@$(MKDIR) -p Obj
	@$(MKDIR) -p Bin

dependents:
	@cd $(PCAPPLUSPLUS_HOME) && $(MAKE) libs
	@cd $(HASH_LIB_HOME) && $(MAKE) hash_library_sources

TLSFingerprinting: start create-directories $(OBJS_FILENAMES)
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_LIBS_DIR) -o "./Bin/TLSFingerprinting$(BIN_EXT)" $(OBJS_FILENAMES) $(PCAPPP_LIBS) -lmysqlcppconn
	@$(PCAPPP_POST_BUILD)
	@echo Finished successfully building: $(CUR_TARGET)
	@echo ' '

clean:
	@cd $(HASH_LIB_HOME) && $(MAKE) clean
	@$(RM) -rf ./Obj/*
	@$(RM) -rf ./Bin/*
	@echo Clean finished: $(CUR_TARGET)

# 附加目标，用于执行gprof分析
profile:
	./$(TARGET)
	gprof $(TARGET) gmon.out > analysis.txt