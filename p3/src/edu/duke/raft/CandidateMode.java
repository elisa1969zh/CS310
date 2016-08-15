package edu.duke.raft;
import java.util.Timer;

public class CandidateMode extends RaftMode {
    private Timer checkVoteTimer;
    private int CHECK_VOTE_TIMER_ID;
    final private int CHECK_VOTE_TIME_INTERVAL = 30;
    private Timer callAnotherElectionTimer;
    private int CALL_ANOTHER_ELECTION_TIMER_ID;
    private boolean callingAnotherElection = false;
    private boolean candidateAlreadySwitchedMode = true;
    
    private void cancelBothTimer () {
        checkVoteTimer.cancel();
        callAnotherElectionTimer.cancel();
    }
    
    private void initiateCheckVoteTimer () {
        CHECK_VOTE_TIMER_ID = 7 * mID * mConfig.getCurrentTerm();
        checkVoteTimer = scheduleTimer(CHECK_VOTE_TIME_INTERVAL, CHECK_VOTE_TIMER_ID);
    }
    
    private void initiateCallAnotherElectionTimer () {
        CALL_ANOTHER_ELECTION_TIMER_ID = 11 * mID * mConfig.getCurrentTerm();
        
        if (mConfig.getTimeoutOverride() <= 0) {callAnotherElectionTimer = scheduleTimer((long) Math.random()*(ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + ELECTION_TIMEOUT_MIN, CALL_ANOTHER_ELECTION_TIMER_ID);}
        else {callAnotherElectionTimer = scheduleTimer(mConfig.getTimeoutOverride(), CALL_ANOTHER_ELECTION_TIMER_ID);}
    }
    
    public void go () {
        synchronized (mLock) {
            // if (previousMode == 'l') {System.out.println("------------------------------PREVIOUS MODE LEADER-----------------------------------------------");}
            mConfig.setCurrentTerm(mConfig.getCurrentTerm() + 1, mID);
            initiateCheckVoteTimer();
            initiateCallAnotherElectionTimer();
            
            RaftResponses.setTerm(mConfig.getCurrentTerm());
            RaftResponses.clearVotes(mConfig.getCurrentTerm());
            
            for (int i = 1; i < mConfig.getNumServers() + 1; i++) {
                // System.out.println("Server " + mID + " asking server " + i + " to vote");
                remoteRequestVote(i, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm());
            }
            
            if (callingAnotherElection) {callingAnotherElection = false;}
            else {System.out.println("S" + mID + "." + mConfig.getCurrentTerm() + ": switched to candidate mode.");}
            
            candidateAlreadySwitchedMode = false;
            
            // previousMode = 'c';
        }
    }
    
    // @param candidate’s term
    // @param candidate requesting vote
    // @param index of candidate’s last log entry
    // @param term of candidate’s last log entry
    // @return 0, if server votes for candidate; otherwise, server's current term
    public int requestVote (int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
        synchronized (mLock) {
            if (candidateID == mID) {return 0;}
            
            if (candidateTerm < mConfig.getCurrentTerm()) {return mConfig.getCurrentTerm();}
            
            if (candidateTerm > mConfig.getCurrentTerm()) {
                if (lastLogTerm >= mLog.getLastTerm()) {
                    cancelBothTimer();
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    
                    if (candidateAlreadySwitchedMode == false) {
                        candidateAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return mConfig.getCurrentTerm();
                }
                else {
                    cancelBothTimer();
                    mConfig.setCurrentTerm(candidateTerm, 0);
                    
                    if (candidateAlreadySwitchedMode == false) {
                        candidateAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return mConfig.getCurrentTerm();
                }
            }
            
            // System.out.println(" - candidate");
            
            if (lastLogTerm >= mLog.getLastTerm()) {
                cancelBothTimer();
                
                if (candidateAlreadySwitchedMode == false) {
                    candidateAlreadySwitchedMode = true;
                    RaftServerImpl.setMode(new FollowerMode());
                }
                
                return mConfig.getCurrentTerm();
            }
            else {
                cancelBothTimer();
                mConfig.setCurrentTerm(candidateTerm, 0);
                callingAnotherElection = true;
                
                if (candidateAlreadySwitchedMode == false) {
                    candidateAlreadySwitchedMode = true;
                    go();
                }
                
                return mConfig.getCurrentTerm();
            }
            
        }
    }
    
    
    // @param leader’s term
    // @param current leader
    // @param index of log entry before entries to append
    // @param term of log entry before entries to append
    // @param entries to append (in order of 0 to append.length-1)
    // @param index of highest committed entry
    // @return 0, if server appended entries; otherwise, server's current term
    public int appendEntries (int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries, int leaderCommit) {
        synchronized (mLock) {
            // System.out.println(entries.length + " " + leaderTerm + " " + term);
            if (leaderTerm < mConfig.getCurrentTerm()) {return mConfig.getCurrentTerm();}
            
            else {
                mConfig.setCurrentTerm(leaderTerm, leaderID);
                cancelBothTimer();
                
                if (candidateAlreadySwitchedMode == false) {
                    candidateAlreadySwitchedMode = true;
                    RaftServerImpl.setMode(new FollowerMode());
                }
                
                return -1;
                // mLog.insert(entries, mLog.getLastIndex(), mLog.getLastTerm());
            }
        }
    }
    
    // @param id of the timer that timed out
    public void handleTimeout (int timerID) {
        synchronized (mLock) {
            // System.out.println("timer triggered -- mID: " + mID + " timerID: " + timerID);
            
            if (timerID == 7 * mID * mConfig.getCurrentTerm()) {
            
                int[] votes = RaftResponses.getVotes(mConfig.getCurrentTerm());
                int count = 0;
                int vote;
                
                for (int i = 1; i < votes.length; i++) {
                    vote = votes[i];
                    
                    if (vote > mConfig.getCurrentTerm()) {
                        // System.out.println("point 3");
                        cancelBothTimer();
                        mConfig.setCurrentTerm(vote, i);
                        
                        if (candidateAlreadySwitchedMode == false) {
                            candidateAlreadySwitchedMode = true;
                            RaftServerImpl.setMode(new FollowerMode());
                        }
                        
                        return;
                    }
                    
                    if (vote == 0) {count++;}  /*System.out.print(vote + " ");*/
                }
                
                // System.out.println();
                
                if (count *2 > mConfig.getNumServers()) {
                    cancelBothTimer();
                    
                    if (candidateAlreadySwitchedMode == false) {
                        candidateAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new LeaderMode());
                    }
                    
                    return;
                }
                else {
                    RaftResponses.clearVotes(mConfig.getCurrentTerm());
                    
                    for (int i = 1; i < mConfig.getNumServers() + 1; i++) {
                        remoteRequestVote(i, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm());
                    }
                    
                    checkVoteTimer.cancel();
                    initiateCheckVoteTimer();
                }
            }
            else if (timerID == 11 * mID * mConfig.getCurrentTerm()) {
                checkVoteTimer.cancel();
                callAnotherElectionTimer.cancel();
                callingAnotherElection = true;
                
                if (candidateAlreadySwitchedMode == false) {
                    candidateAlreadySwitchedMode = true;
                    go();
                }
            }
            
            // else {System.out.println(mID + ": wrong timer");}
        }
    }
}
