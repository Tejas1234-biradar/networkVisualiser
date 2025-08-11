CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LIBS = -lpcap -pthread
TARGET = packet_sniffer
SOURCES = main.cpp packetSniffer.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean