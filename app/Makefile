src = $(wildcard *.c)
obj = $(src:.c=.o)
main: $(obj)
	$(CC) -o $@ $^
.PHONY: clean
clean:
	@echo "Lets clean it..."
	rm -f $(obj) main