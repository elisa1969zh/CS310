RAFT_PATH=edu/duke/raft
SRC_DIR=src/$(RAFT_PATH)
JC = javac
JFLAGS = -d ./bin -classpath ./bin:./src

.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = \
	$(SRC_DIR)/CandidateMode.java \
	$(SRC_DIR)/Entry.java \
	$(SRC_DIR)/FollowerMode.java \
	$(SRC_DIR)/LeaderMode.java \
	$(SRC_DIR)/RaftConfig.java \
	$(SRC_DIR)/RaftLog.java \
	$(SRC_DIR)/RaftMode.java \
	$(SRC_DIR)/RaftResponses.java \
	$(SRC_DIR)/RaftServer.java \
	$(SRC_DIR)/RaftServerImpl.java \
	$(SRC_DIR)/StartClient.java \
	$(SRC_DIR)/StartServer.java

default: classes

classes: $(CLASSES:.java=.class)

clean:
	$(RM) *.class

# r: 	
	# find -exec touch \{\} \; && cd bin && ./testing1.sh && find -exec touch \{\} \; && cd bin && ./testing2.sh && find -exec touch \{\} \; && cd bin && ./testing3.sh

r1:
	find -exec touch \{\} \; && cd bin && ./testing1.sh

r2:
	find -exec touch \{\} \; && cd bin && ./testing2.sh

r3:
	find -exec touch \{\} \; && cd bin && ./testing3.sh

test1:
	bin/runtest.sh 5 tests/sampletest1.txt 2> /dev/null

test2:
	bin/runtest.sh 5 tests/sampletest2.txt 2> /dev/null

test3:
	bin/runtest.sh 5 tests/sampletest3.txt 2> /dev/null

c:
	find -exec touch \{\} \;

diff1_2:
	cd serverlogs && diff 1.log 2.log

diff2_3:
	cd serverlogs && diff 2.log 3.log
	
diff3_1:
	cd serverlogs && diff 3.log 1.log

cat1:
	cd serverlogs && cat 1.log
cat2:
	cd serverlogs && cat 2.log
cat3:
	cd serverlogs && cat 3.log

commit:
	cd .. && git add --all && git commit -a -m 'p3'