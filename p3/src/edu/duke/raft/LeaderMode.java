package edu.duke.raft;
import java.util.Timer;

public class LeaderMode extends RaftMode {
    private int SEND_HEARTBEAT_MESSAGE_TIMER_ID;
    private Timer sendHeartbeatMessageTimer;
    private int[] nextIndex = new int[mConfig.getNumServers() + 1];
    private boolean leaderAlreadySwitchedMode = true;
    
    private void initiateSendHeartbeatMessageTimer () {
        SEND_HEARTBEAT_MESSAGE_TIMER_ID = 13 * mID * mConfig.getCurrentTerm();
        sendHeartbeatMessageTimer = scheduleTimer(HEARTBEAT_INTERVAL, SEND_HEARTBEAT_MESSAGE_TIMER_ID);
    }
    
    public void sendAppendEntriesCommand () {
        synchronized (mLock) {
            for (int i = 1; i < mConfig.getNumServers() + 1; i++) {
                // System.out.println("nextIndex[" + i + "] = " + nextIndex[i]);
                Entry[] entries;
                
                if (mLog.getEntry(nextIndex[i]) == null) {entries = new Entry[0];}
                else {
                    entries = new Entry[mLog.getLastIndex() - nextIndex[i] + 1];
                    
                    for (int j = nextIndex[i]; j <= mLog.getLastIndex(); j++) {entries[j - nextIndex[i]] = mLog.getEntry(j);}
                }
                
                remoteAppendEntries(i, mConfig.getCurrentTerm(), mID, nextIndex[i] -1, mLog.getEntry(nextIndex[i] -1).term, entries, mCommitIndex);
            }
        }
    }
    
    public void go () {
        synchronized (mLock) {
            initiateSendHeartbeatMessageTimer();
            
            for (int j = 1; j < mConfig.getNumServers() + 1; j++) {nextIndex[j] = mLog.getLastIndex() + 1;}
            
            RaftResponses.clearAppendResponses(mConfig.getCurrentTerm());
            sendAppendEntriesCommand();
            System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": switched to leader mode.");
            leaderAlreadySwitchedMode = false;
        }
    }
    
    public int requestVote (int candidateTerm, int candidateID, int lastLogIndex, int lastLogTerm) {
        synchronized (mLock) {
            // System.out.println("got vote!");
            if (candidateTerm < mConfig.getCurrentTerm()) {return mConfig.getCurrentTerm();}
            
            if (candidateTerm > mConfig.getCurrentTerm()) {
                if (lastLogTerm >= mLog.getLastTerm()) {
                    sendHeartbeatMessageTimer.cancel();
                    mConfig.setCurrentTerm(candidateTerm, candidateID);
                    
                    if (leaderAlreadySwitchedMode == false) {
                        leaderAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return 0;
                }
                else {
                    sendHeartbeatMessageTimer.cancel();
                    mConfig.setCurrentTerm(candidateTerm, 0);
                    
                    if (leaderAlreadySwitchedMode == false) {
                        leaderAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return mConfig.getCurrentTerm();
                }
            }
            
            
            if (lastLogTerm >= mLog.getLastTerm()) {
                if (mConfig.getVotedFor() == 0 || mConfig.getVotedFor() == candidateID) {
                    // System.out.println(mID + " - Leader");
                    sendHeartbeatMessageTimer.cancel();
                    
                    if (leaderAlreadySwitchedMode == false) {
                        leaderAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return 0;
                }
                else {
                    sendHeartbeatMessageTimer.cancel();
                    
                    if (leaderAlreadySwitchedMode == false) {
                        leaderAlreadySwitchedMode = true;
                        RaftServerImpl.setMode(new FollowerMode());
                    }
                    
                    return mConfig.getCurrentTerm();
                }
            }
            
            else {return mConfig.getCurrentTerm();}
        }
    }
    
    public int appendEntries (int leaderTerm, int leaderID, int prevLogIndex, int prevLogTerm, Entry[] entries, int leaderCommit) {
        synchronized (mLock) {
            if (leaderTerm > mConfig.getCurrentTerm ()) {
                sendHeartbeatMessageTimer.cancel();
                mConfig.setCurrentTerm(leaderTerm, 0);
                
                if (leaderAlreadySwitchedMode == false) {
                    leaderAlreadySwitchedMode = true;
                    RaftServerImpl.setMode(new FollowerMode());
                }
                
                return -1;
            }
            
            if (leaderTerm < mConfig.getCurrentTerm ()) {return mConfig.getCurrentTerm();}
            
            if (leaderID == mID) {return -1;}
            else {
                // System.out.println(mID + ": DOUBLE LEADER!");
                
                if (entries.length == 0) {
                    if (prevLogTerm >= mLog.getLastTerm()) {
                        sendHeartbeatMessageTimer.cancel();
                        
                        if (leaderAlreadySwitchedMode == false) {
                            leaderAlreadySwitchedMode = true;
                            RaftServerImpl.setMode(new FollowerMode());
                        }
                        
                        return -1;
                    }
                    
                    else {
                        mConfig.setCurrentTerm(mConfig.getCurrentTerm() + 1, mID);
                        return mConfig.getCurrentTerm();
                    }
                }
                
                else {
                    if (prevLogTerm > mLog.getLastTerm()) {
                        sendHeartbeatMessageTimer.cancel();
                        
                        if (leaderAlreadySwitchedMode == false) {
                            leaderAlreadySwitchedMode = true;
                            RaftServerImpl.setMode(new FollowerMode());
                        }
                        
                        return -1;
                    }
                    else {
                        return 0;
                    }
                }
            }
        }
    }
    
    // @param id of the timer that timed out
    public void handleTimeout (int timerID) {
        synchronized (mLock) {
            // System.out.println("timer triggered -- mID: " + mID + " timerID: " + timerID);
            
            if (timerID == 13 * mID * mConfig.getCurrentTerm()) {
                sendHeartbeatMessageTimer.cancel();
                initiateSendHeartbeatMessageTimer();
                int[] followerResponses = RaftResponses.getAppendResponses(mConfig.getCurrentTerm());
                
                for (int j = 1; j < followerResponses.length; j++) {
                    // System.out.println(mID + ": Server " + j + "'s response: " + followerResponses[j] + " nextIndex: " + nextIndex[j]);
                    
                    if (followerResponses[j] > 0) {
                        if (followerResponses[j] > mConfig.getCurrentTerm()) {
                            sendHeartbeatMessageTimer.cancel();
                            mConfig.setCurrentTerm(followerResponses[j], 0);
                            
                            if (leaderAlreadySwitchedMode == false) {
                                leaderAlreadySwitchedMode = true;
                                RaftServerImpl.setMode(new FollowerMode());
                            }
                            
                            return;
                        }
                        
                        // else {System.out.println(mID + " I indeed have a higher or equal term as you do");}
                    }
                    
                    switch (followerResponses[j]) {
                    case 0:
                        nextIndex[j] = mLog.getLastTerm() + 1; break;
                        
                    case -1: break;
                    
                    case -2:
                        // if (nextIndex[j] < 1) {System.out.println(mID + ": SOMETHING IS WRONG!");}
                        
                        nextIndex[j] -= 1;
                        break;
                        
                    case -3: break;
                    
                    default: break;
                    }
                }
                
                RaftResponses.clearAppendResponses(mConfig.getCurrentTerm());
                sendAppendEntriesCommand();
            }
        }
    }
}
