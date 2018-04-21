all dep clean indent tests::
	cd onedim && $(MAKE) $@ && cd .. 
	
	
doc: indent doxy

clean::
	rm -rf *~ PI* core bin/* obj/* tmp *.log
