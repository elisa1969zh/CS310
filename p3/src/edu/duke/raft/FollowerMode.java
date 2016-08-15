package edu.duke.raft;
import java.util.Timer;

public class FollowerMode extends RaftMode {
    private Timer checkFailureTimer;
    private int CHECK_FAILURE_TIMER_ID;
    private int commitIndex = 0;
    private boolean followerAlreadySwitchedMode = true;
    
    private void initiateCheckFailureTimer () {
        synchronized (mLock) {
            CHECK_FAILURE_TIMER_ID = 19 * mID * mConfig.getCurrentTerm();
            
            if (mConfig.getTimeoutOverride() <= 0) {
                checkFailureTimer = scheduleTimer((long) Math.random()*(ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN) + ELECTION_TIMEOUT_MIN, CHECK_FAILURE_TIMER_ID);
            }
            else {
                checkFailureTimer = scheduleTimer(mConfig.getTimeoutOverride(), CHECK_FAILURE_TIMER_ID);
                // System.out.println(mID + ": " + mConfig.getTimeoutOverride());
            }
            
            // System.out.println("timer ID initiated: " + CHECK_FAILURE_TIMER_ID);
        }
    }
    
    public void go () {
        synchronized (mLock) {
            initiateCheckFailureTimer();
            System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": switched to follower mode.");
            followerAlreadySwitchedMode = false;
        }
    }
    
    // @param candidate’s term
    // @param candidate requesting vote
    // @param index of candidate’s last log entry
    // @param term of candidate’s last log entry
    // @return 0, if server votes for candidate; otherwise, server's
    // current term
    public int requestVote (int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
        synchronized (mLock) {
            if (candidateTerm < mConfig.getCurrentTerm()) {return mConfig.getCurrentTerm();}
            
            if (candidateTerm > mConfig.getCurrentTerm()) {
                if (lastLogTerm >= mLog.getLastTerm()) {
                    checkFailureTimer.cancel();
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    initiateCheckFailureTimer();
                    // System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": updated to follower mode.");
                    return 0;
                }
                else {
                    checkFailureTimer.cancel();
                    mConfig.setCurrentTerm(candidateTerm, 0);
                    initiateCheckFailureTimer();
                    // System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": updated to follower mode with higher log.");
                    return mConfig.getCurrentTerm();
                }
            }
            
            if ((mConfig.getVotedFor() == candidateID || mConfig.getVotedFor() == 0)) {
            
                if (lastLogTerm >= mLog.getLastTerm()) {
                    checkFailureTimer.cancel();
                    initiateCheckFailureTimer();
                    return 0;
                }
                else {
                    checkFailureTimer.cancel();
                    mConfig.setCurrentTerm(candidateTerm, mID);
                    // System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": jump ahead to candidate mode.");
                    
                    if (followerAlreadySwitchedMode == false) {
                        followerAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new CandidateMode());
                    }
                    
                    return mConfig.getCurrentTerm();
                }
            }
            else {
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
    // @return 0, if server appended entries; otherwise, server's
    // current term
    public int appendEntries (int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries, int leaderCommit) {
        synchronized (mLock) {
            if (leaderTerm < mConfig.getCurrentTerm()) {return mConfig.getCurrentTerm();}           /*CASE 1*/
            
            if (leaderTerm > mConfig.getCurrentTerm()) {
                mConfig.setCurrentTerm(leaderTerm, 0);
                // System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": updated term as follower.");
            }
            
            if (mLog.getEntry(prevLogIndex) == null || mLog.getEntry(prevLogIndex).term != prevLogTerm) {       /*CASE 2*/
                checkFailureTimer.cancel();
                initiateCheckFailureTimer();
                return -2;
            }
            
            else {
                mLog.insert(entries, prevLogIndex, prevLogTerm);
                
                if (leaderCommit > commitIndex) {
                    if (leaderCommit > mLog.getLastIndex()) {commitIndex = mLog.getLastIndex();}
                    else {commitIndex = leaderCommit;}
                }
                
                checkFailureTimer.cancel();
                initiateCheckFailureTimer();
                
                // for (int i = 0; i < mLog.getLastIndex() +1; i++) {
                //     System.out.print("{" + mLog.getEntry(i).toString() + "} ");
                // }
                
                // System.out.println();
                
                if (entries.length == 0) {return -3;}
                else {return 0;}
            }
            
            /*CASE 5*/
            // if (mLog.getEntry(prevLogIndex + 1) == null) {mLog.insert(entries, prevLogIndex, prevLogTerm);}       /*CASE 4*/
            // if (entries.length != 0 && (mLog.getEntry(prevLogIndex + 1) == null || mLog.getEntry(prevLogIndex + 1).term != entries[0].term)
            //         || entries.length == 0 && prevLogIndex < mLog.getLastIndex()) {                                               /*CASE 3*/
            // System.out.println(mID + "'s previous Entry = {" + mLog.getEntry(prevLogIndex) + "}");
            // if (mlog.getEntry(prevLogIndex+1) != null) {
            //     for (int i = prevLogIndex+1; i<mlog.getLastIndex()+1; i++) {
            //         Entry[] e = new Entry[0];
            //         mlog.insert(e, i, mConfig.getCurrentTerm());
            //     }
            // }
            // }
            // System.out.print(mID + "'s log: ");
            
            // System.out.println("Server " + mID + ": heartbeat message received!");
            // System.out.println("point 3, {" + prevLogIndex + "}");
            
        }
    }
    
// @param id of the timer that timed out
    public void handleTimeout (int timerID) {
        synchronized (mLock) {
            if (timerID == mID * mConfig.getCurrentTerm() * 19) {
                checkFailureTimer.cancel();
                // System.out.println("timer triggered -- expected ID: " + mID * mConfig.getCurrentTerm() * 19 + "; TIMER_ID:" + timerID);
                
                if (followerAlreadySwitchedMode == false) {
                    followerAlreadySwitchedMode = true;
                    RaftServerImpl.setMode(new CandidateMode());
                }
            }
            
            // else {
            // System.out.println(mID + ": timer rejected!");
            // }
        }
    }
}

