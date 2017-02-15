all:
	make -C trolley-interface
	make -C vme-interface
	make -C yokogawa-interface	

%:
	make $@ -C trolley-interface
	make $@ -C vme-interface
	make $@ -C yokogawa-interface