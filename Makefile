probe:
	g++ -std=c++17 -O2 hashtable.cpp -o probe

badprobe:
	g++ -std=c++17 -O2 hashtable.cpp -o badprobe -DBAD_ACCESS_KEY

chain:
	g++ -std=c++17 -O2 hashtable_chain.cpp -o chain

badchain:
	g++ -std=c++17 -O2 hashtable_chain.cpp -o badchain -DBAD_ACCESS_KEY
