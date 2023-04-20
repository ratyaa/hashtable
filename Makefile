probe:
	g++ -std=c++17 -O2 hashtable.cpp -o probe

badprobe:
	g++ -std=c++17 -O2 hashtable.cpp -o probe -DBAD_ACCESS_KEY

chain:
	g++ -std=c++17 -O2 hashtable_chain.cpp -o chain

badchain:
	g++ -std=c++17 -O2 hashtable_chain.cpp -o chain -DBAD_ACCESS_KEY
