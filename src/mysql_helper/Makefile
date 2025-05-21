# project name
project_name               := $(shell basename "$(CURDIR)")



# paths home
project_home                = .
install_dir                 = $(project_home)/../../include


# includes of project headers
project_inc_path            = $(project_home)



# build targets
targets = project

# let 'build' be default target, build all targets
build   : $(targets)

project :
	@echo
	@echo "@@@@@  start making $(project_name)  @@@@@"
	@echo "do nothing"
	@echo "@@@@@  make $(project_name) success  @@@@@"
	@echo

clean   :
	@echo

rebuild : clean build

install :
	if [ ! -d $(install_dir) ]; then	\
		mkdir -p $(install_dir);		\
	fi
	cp -f $(project_inc_path)/$(project_name).hpp $(install_dir)/
