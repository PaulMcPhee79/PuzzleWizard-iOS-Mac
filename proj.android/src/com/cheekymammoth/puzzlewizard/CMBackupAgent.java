package com.cheekymammoth.puzzlewizard;

import android.app.backup.BackupAgentHelper;
import android.app.backup.BackupManager;
import android.app.backup.FileBackupHelper;

// This is NOT threadsafe! See developer.android.com/guide/topics/data/backup.html
@SuppressWarnings("unused")
public class CMBackupAgent extends BackupAgentHelper {
//	private static final String kBackupFileName = "PW_GameProgress.dat";
//	private static final String kBackupKey = "PW_GameProgress";
	
	@Override
	public void onCreate() {
//		FileBackupHelper helper = new FileBackupHelper(this, kBackupFileName);
//		addHelper(kBackupKey, helper);
	}
	
	public void requestBackup() {
//		BackupManager bm = new BackupManager(this);
//		bm.dataChanged();
	}
}
