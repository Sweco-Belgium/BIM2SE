all: generate
	

generate:
	@echo "Configure, build and run the script"
	@echo "-CONFIGURE ---------" && ./tools/configure.sh 
	@echo "-BUILD -------------" && ./tools/build.sh
	@echo "-RUN ---------------" && ./tools/run.sh

clean:
	@echo "Removing the build directory..."
	rm -rf ./build
	rm *.stl
	rm *.stp
