/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package com.cheekymammoth.puzzlewizard;

import com.cheekymammoth.puzzlewizard.R;
//import com.crashlytics.android.Crashlytics;
//import com.flurry.android.FlurryAgent;
//import com.google.android.vending.licensing.AESObfuscator;
//import com.google.android.vending.licensing.LicenseChecker;
//import com.google.android.vending.licensing.LicenseCheckerCallback;
//import com.google.android.vending.licensing.LicenseValidator;
//import com.google.android.vending.licensing.Policy;
////import com.google.android.vending.licensing.ServerManagedPolicy;
//import com.google.android.vending.licensing.StrictPolicy;

import org.cocos2dx.lib.Cocos2dxActivity;

import android.view.View;
import android.view.Window;
import android.view.WindowManager;
//import android.widget.Toast;
import android.util.DisplayMetrics;
import android.util.Log;

//import java.util.HashMap;
//import java.util.Map;
import java.util.Locale;

import net.hockeyapp.android.Constants;
import net.hockeyapp.android.CrashManager;
import net.hockeyapp.android.UpdateManager;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings.Secure;

public class puzzlewizard extends Cocos2dxActivity{
	private static final String TAG = "PuzzleWizard";
	private static final String HOCKEYAPP_ID = "2bfdb5a94923b44d5c3b6864d094ec52";
	
	private static puzzlewizard s_wizard = null;
	private static CMEncrypter s_encrypter = null;
	
	// Google licensing
	private static final String GPFU = "uFheTroFyaPts";
    private static final String GPAH = "sA,emaGgnikc";
	private Handler mHandler;
//	private LicenseChecker mChecker;
//	private LicenseCheckerCallback mLicenseCheckerCallback;
	
	private static final int kLicenseInProgress = -1;
	private static final int kLicenseDenied = 0;
	private static final int kLicenseApproved = 1;
	private static final int kLicenseProvisionalRetry = 2;
	private static final int kLicenseProvisionalError = 3;
	private static final int kLicenseDialogOK = 4;
	private static final int kLicenseDialogRetry = 5;
	private static final int kLicenseDialogBuy = 6;
	
//	static {  
//	    System.loadLibrary("CMNative");  
//	}
	
	private volatile int licensed = kLicenseApproved; //licensed = kLicenseProvisionalRetry;
	private volatile int licensedExpirationMins = 90;
	
	// Google backup
	private CMBackupAgent backupAgent;
	
	private native void setUpBreakpad(String filepath);
	private native void nativeCrash();

	protected void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		
		//Crashlytics.start(this);

		s_wizard = this;
		//View sb = getWindow().getDecorView();
		//sb.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
		
		if (backupAgent == null)
			backupAgent = new CMBackupAgent();
		
		Window window = getWindow();
    	if (window != null)
    		window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON | WindowManager.LayoutParams.FLAG_FULLSCREEN);

    	//setContentView(R.layout.activity_main);

        // Handle NDK crashes
        Constants.loadFromContext(this);
        setUpBreakpad(Constants.FILES_PATH);
        NativeCrashManager.handleDumpFiles(this, HOCKEYAPP_ID);
    	
    	checkForUpdates();
    	//Log.v(TAG + ": FILES DIR", new File(this.getFilesDir(), "PW_GameProgress.dat").getAbsolutePath());
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
//		FlurryAgent.setCaptureUncaughtExceptions(false);
//		FlurryAgent.onStartSession(this, "MQHPK28TXH8F62XP47R3");
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		UpdateManager.unregister();
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		checkForCrashes();
	}
	
	@Override
	protected void onStop()
	{
		super.onStop();		
//		FlurryAgent.onEndSession(this);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		
//		if (mChecker != null)
//			mChecker.onDestroy();
	}
	
//	@Override
//	public void onBackPressed() {
//		
//	}
	
	public void onNativeCrashClicked(View v) {
		nativeCrash();
	}
	  
	public void onCrashClicked(View v) {
		View view = findViewById(0x123);
		view.setVisibility(View.INVISIBLE);
	}
	
	private void checkForCrashes() {
		CrashManager.register(this, HOCKEYAPP_ID);
	}
	
	private void checkForUpdates() {
		// Remove this for store / production builds!
//		UpdateManager.register(this, "2bfdb5a94923b44d5c3b6864d094ec52");
	}
	
	public String generateGooglePlayLicenseKey() {
		String deviceId = Secure.getString(getContentResolver(), Secure.ANDROID_ID);
		if (deviceId == null)
			return "p.ffgfglkhjIOgdasgfsdf645ydfg23r[erp_9iOp"; 
		else
			return "Junk_98tnfFs" + deviceId + "p.few-5sa4set"; 
	}
	
	public boolean validateGooglePlayLicenseKey(String licenseKey) {
		if (licenseKey == null)
			return false;
		else
		{
			String generatedKey = generateGooglePlayLicenseKey();
			String decryptedLicenseKey = puzzlewizard.markMasterPuzzler(licenseKey);
			return generatedKey.equalsIgnoreCase(decryptedLicenseKey);
		}
	}
	
	public int getLicensed() {
		return licensed;
	}
	
	public void setLicensed(int value) {
		//licensed = value;
		licensed = kLicenseApproved; // Licensing disabled
	}
	
	public int getLicensedExpirationMins() {
		return licensedExpirationMins;
	}
	
	public void setLicensedExpirationMins(int value) {
		licensedExpirationMins = Math.max(0, value);
	}
	
	public void performGooglePlayLicenseCheck() {
		switch (licensed) {
			case kLicenseInProgress:
			case kLicenseProvisionalRetry:
			case kLicenseProvisionalError:
			case kLicenseDialogOK:
			case kLicenseDialogRetry:
			case kLicenseDialogBuy:
			{
				licensed = kLicenseInProgress;
				this.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						if (mHandler == null) {
							mHandler = new Handler();
//							mLicenseCheckerCallback = new CMLicenseChecker();
							
							//String deviceId = Secure.getString(getContentResolver(), Secure.ANDROID_ID);
							//Log.i(TAG, deviceId);

//							mChecker = new LicenseChecker(s_wizard,
//									//new ServerManagedPolicy(s_wizard, new AESObfuscator(CMEncrypter.getSalt(), getPackageName(), deviceId)),
//									new StrictPolicy(),
//									CMEncrypter.getPK());
						}
						
						doCheck();
					}
				});
			}
			default:
				break;
		}
	}

	public void requestBackup() {
		if (backupAgent == null)
			backupAgent = new CMBackupAgent();
		backupAgent.requestBackup();
	}
	
    public void enableKeepScreenOn(boolean enable) {
    	Window window = getWindow();
    	if (window != null) {
    		if (enable)
    			window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    		else
    			window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    	}
	}
	
    static {
         System.loadLibrary("game");
    }
    
    public static void backupGameProgress() {
    	if (s_wizard != null)
    		s_wizard.requestBackup();
    }
    
    public static void openMarketPlaceToRateTheGame() {
    	Context context = Cocos2dxActivity.getContext();
    	if (context != null) {
	    	String urlString = "market://details?id=" + context.getPackageName();
	    	//Log.w(TAG + ": APP URL", urlString);
	    	Uri uri = Uri.parse(urlString);
	    	Intent goToMarket = new Intent(Intent.ACTION_VIEW, uri);
	    	try {
	    		context.startActivity(goToMarket);
	    	} catch (ActivityNotFoundException e) {
	    		// Ignore
	    	}
    	}
    }
    
    public static void openURL(String url) {
    	if (url != null) {
    		Context context = Cocos2dxActivity.getContext();
    		if (context != null) {
    			Uri uri = Uri.parse(url);
    			Intent openURI = new Intent(Intent.ACTION_VIEW, uri);
    			try {
    				context.startActivity(openURI);
    			} catch (ActivityNotFoundException e) {
    				// Ignore
    			}
    		}
    	}
    }
    
    public static int getDefaultLocale() {
    	String localeCode = Locale.getDefault().toString();
    	if (localeCode != null && localeCode.length() >= 2) {
    		String isoCode = localeCode.substring(0, 2);
    		if (isoCode.equalsIgnoreCase("zh"))
    			return 1;
    		else if (isoCode.equalsIgnoreCase("de"))
    			return 2;
    		else if (isoCode.equalsIgnoreCase("es"))
    			return 3;
    		else if (isoCode.equalsIgnoreCase("fr"))
    			return 4;
    		else if (isoCode.equalsIgnoreCase("it"))
    			return 5;
    		else if (isoCode.equalsIgnoreCase("ja"))
    			return 6;
    		else if (isoCode.equalsIgnoreCase("ko"))
    			return 7;
    		else
    			return 0; // "en"
    	} else {
    		return 0; // "en"
    	}
    }
    
    public static int getDisplayWidthInPixels() {
    	if (s_wizard == null)
    		return 0;
    	else {
    		DisplayMetrics dm = new DisplayMetrics();
    		s_wizard.getWindowManager().getDefaultDisplay().getMetrics(dm);
    		return dm.widthPixels;
    	}
    }

	public static int getDisplayHeightInPixels() {
		if (s_wizard == null)
    		return 0;
    	else {
    		DisplayMetrics dm = new DisplayMetrics();
    		s_wizard.getWindowManager().getDefaultDisplay().getMetrics(dm);
    		return dm.heightPixels;
    	}
	}
    
    public static float getDisplayXdpi() {
    	if (s_wizard == null)
    		return 0;
    	else {
    		DisplayMetrics dm = new DisplayMetrics();
    		s_wizard.getWindowManager().getDefaultDisplay().getMetrics(dm);
    		return dm.xdpi;
    	}
    }
    
    public static float getDisplayYdpi() {
    	if (s_wizard == null)
    		return 0;
    	else {
    		DisplayMetrics dm = new DisplayMetrics();
    		s_wizard.getWindowManager().getDefaultDisplay().getMetrics(dm);
    		return dm.ydpi;
    	}
    }
    
    public static String getLocaleFromResStrings() {
    	Context context = Cocos2dxActivity.getContext();
    	if (context != null)
    		return context.getString(R.string.CM_LocaleKey);
    	return "EN";
    }
    
    public static void keepScreenOn() {
    	if (s_wizard != null)
    		s_wizard.enableKeepScreenOn(true);
    }

    public static void allowScreenOff() {
    	if (s_wizard != null)
    		s_wizard.enableKeepScreenOn(false);
    }

    private static CMEncrypter getEncrypter() {
    	if (s_encrypter == null)
    	{
    		try {
    			s_encrypter = new CMEncrypter(Cocos2dxActivity.getContext(), "elohs" + GPAH + GPFU + "uJesaelP");
    		} catch (Exception e) {
    			Log.i(TAG + ": CME","Failed to initialize.");
    		}
    	}
    	
    	return s_encrypter;
    }
    
    // encryptString
    public String addToRewards_(String value) {
    	CMEncrypter encrypter = getEncrypter();
    	if (encrypter != null)
    	{
    		try {
    			return encrypter.encryptValue(value);
    		} catch (CMEncrypter.CMEncrypterException e) {
    			Log.i(TAG + ": CME","encryption failed.");
    		}
    	}
    	
    	return value;
    }
    public static String addToRewards(String value) {
    	if (s_wizard != null)
    		return s_wizard.addToRewards_(value);
    	return value;
    }
    
    // decryptString
    public String markMasterPuzzler_(String value) {
    	CMEncrypter encrypter = getEncrypter();
    	if (encrypter != null)
    	{
    		try {
    			return encrypter.decryptValue(value);
    		} catch (CMEncrypter.CMEncrypterException e) {
    			Log.i(TAG + ": CME","decryption failed.");
    		}
    	}

    	return value;
    }
    public static String markMasterPuzzler(String value) {
    	if (s_wizard != null)
    		return s_wizard.markMasterPuzzler_(value);
    	return value;
    }
    
    // Flurry
    public static void FL_logEvent(String ev) {
//    	if (ev != null)
//    		FlurryAgent.logEvent(ev);
    }
    
    public static void FL_logEvent(String ev, String[] params) {
//    	if (ev != null && params != null && params.length > 0 && params.length % 2 == 0) {
//    		Map<String, String> FL_params = new HashMap<String, String>();
//    		for (int i = 0, n = params.length-1; i < n; i+=2)
//    			FL_params.put(params[i], params[i+1]);
//    		FlurryAgent.logEvent(ev, FL_params);
//    	}
    }

    // Google licensing
    
    // doGooglePlayLicenseCheck
    public void congratulateUser_() {
    	performGooglePlayLicenseCheck();
    }
    public static void congratulateUser() {
    	if (s_wizard != null)
    		s_wizard.congratulateUser_();
    }
    
    // getGooglePlayLicenseStatus
    public int getPuzzleCompletedCount_() {
    	return getLicensed();
    }
    public static int getPuzzleCompletedCount() {
    	return s_wizard != null ? s_wizard.getPuzzleCompletedCount_() : 1;
    }
    
    // setGooglePlayLicenseExpiration
    public void setStatLongestPuzzleDuration_(int minsRemaining) {
    	setLicensedExpirationMins(minsRemaining);
    }
    public static void setStatLongestPuzzleDuration(int minsRemaining) {
    	if (s_wizard != null)
    		s_wizard.setStatLongestPuzzleDuration_(minsRemaining);
    }
    
    // verifyGooglePlayLicenseKey
    public boolean isPlayerMasterPuzzler_(String licenseKey) {
    	if (licenseKey != null && licenseKey.isEmpty() == false)
			return validateGooglePlayLicenseKey(licenseKey);
		else
			return false;
    }
    public static boolean isPlayerMasterPuzzler(String licenseKey) {
    	if (s_wizard != null)
    		return s_wizard.isPlayerMasterPuzzler_(licenseKey);
    	else
    		return true;
    }
    
    // getGooglePlayLicenseKey
    public String getMasterPuzzlerKey_() {
    	return addToRewards(generateGooglePlayLicenseKey());
    }
    public static String getMasterPuzzlerKey() {
    	if (s_wizard != null)
    		return s_wizard.getMasterPuzzlerKey_();
    	else
    		return null;
    }
    
    private void doCheck() {
        //setProgressBarIndeterminateVisibility(true);
//    	Toast.makeText(s_wizard, s_wizard.getString(R.string.ToastLicenseChecking), Toast.LENGTH_LONG).show();
//        mChecker.checkAccess(mLicenseCheckerCallback);
    }
    
    public static void showGooglePlayLicenseDialog(String dialogType) {
//    	if (s_wizard != null)
//    	{
//    		s_wizard.setLicensed(kLicenseInProgress);
//    		if (dialogType.equalsIgnoreCase("OK_RETRY"))
//    			LicenseDialog.type = LicenseDialog.LicenseDialogType.OK_RETRY;
//    		else // if (dialogKey.equalsIgnoreCase("BUY_RETRY"))
//    			LicenseDialog.type = LicenseDialog.LicenseDialogType.BUY_RETRY;
//    		new LicenseDialog().show(s_wizard.getFragmentManager(), "License Dialog");
//    	}
    }
    
//    private class CMLicenseChecker implements LicenseCheckerCallback {
//    	
//        @Override
//        public void allow(int reason) {
//            if (isFinishing()) {
//                // Don't update UI if Activity is finishing.
//                return;
//            }
//            
//            Toast.makeText(s_wizard, "License Approved!", Toast.LENGTH_LONG).show();
//            Log.i(TAG, "License Accepted!");
//            licensed = kLicenseApproved;
//        }
//
//        @Override
//        public void dontAllow(int reason, int serverResponse) {
//        	if (isFinishing()) {
//            	 // Don't update UI if Activity is finishing.
//            	 return;
//            }
//        	
//        	switch (serverResponse) {
//	        	case LicenseValidator.ERROR_CONTACTING_SERVER:
//					licensed = kLicenseProvisionalRetry;
//			 		Toast.makeText(s_wizard, "License Error Contacting Server [retry].", Toast.LENGTH_LONG).show();
//					break;
//				case LicenseValidator.ERROR_SERVER_FAILURE:
//					licensed = kLicenseProvisionalRetry;
//			 		Toast.makeText(s_wizard, "License Error Server Failure [retry].", Toast.LENGTH_LONG).show();
//					break;
//				case LicenseValidator.ERROR_OVER_QUOTA:
//					licensed = kLicenseDenied;
//			 		Toast.makeText(s_wizard, "License Error Over Quota [denied].", Toast.LENGTH_LONG).show();
//					break;
//				default:
//				{
//					switch (reason) {
//					 	case Policy.NOT_LICENSED:
//					 		licensed = kLicenseDenied;
//					 		Toast.makeText(s_wizard, "License Denied!", Toast.LENGTH_LONG).show();
//					 		break;
//					 	case Policy.RETRY:
//					 		licensed = kLicenseProvisionalRetry;
//					 		Toast.makeText(s_wizard, "License Error [retry].", Toast.LENGTH_LONG).show();
//					 		break;
//					 	default:
//					 		break;
//					 }
//				}
//				break;
//        	}
//        	
//        	Log.i(TAG, "License Denied!");
//        	Log.i(TAG, "Reason for denial: "+reason);
//        }
//
//        @Override
//        public void applicationError(int reason) {
//            Log.i(TAG, "License Error: " + reason);
//            if (isFinishing()) {
//                // Don't update UI if Activity is finishing.
//                return;
//            }
//            
//            switch (reason) {
//            	case LicenseCheckerCallback.ERROR_CHECK_IN_PROGRESS:
//            	{
//            		licensed = kLicenseInProgress;
//            	}
//            		break;
//            	default:
//            	{
//            		licensed = kLicenseProvisionalError;
//            	}
//            		break;
//            }
//            
//            Toast.makeText(s_wizard, "License Error!", Toast.LENGTH_LONG).show();
//        }
//    }
    
    public static class LicenseDialog extends DialogFragment {
    	public enum LicenseDialogType { OK_RETRY, BUY_RETRY };
    	public static LicenseDialogType type = LicenseDialogType.OK_RETRY;
    	@Override
    	public Dialog onCreateDialog(Bundle savedInstanceState) {
    		switch (type) {
    			case BUY_RETRY:
    			{
    				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
    	    		alertDialogBuilder.setTitle(s_wizard.getString(R.string.DlgTitleLicenseFailed));
    	    		alertDialogBuilder.setMessage(s_wizard.getString(R.string.MsgLicenseExpired));
    	    		alertDialogBuilder.setPositiveButton(s_wizard.getString(R.string.DlgBtnRetry), new DialogInterface.OnClickListener() {
    	    			
    					@Override
    					public void onClick(DialogInterface dialog, int which) {
    						s_wizard.setLicensed(kLicenseDialogRetry);
    						//Toast.makeText(s_wizard, s_wizard.getString(R.string.ToastLicenseChecking), Toast.LENGTH_LONG).show();
    						dismiss();
    					}
    				});
    	    		alertDialogBuilder.setNegativeButton(s_wizard.getString(R.string.DlgBtnBuyNow), new DialogInterface.OnClickListener() {

    					@Override
    					public void onClick(DialogInterface dialog, int which) {
    						s_wizard.setLicensed(kLicenseDialogBuy);
    						puzzlewizard.openMarketPlaceToRateTheGame();
    						dismiss();
    					}
    				});
    	    		
    	    		setCancelable(false);
    	    		return alertDialogBuilder.create();
    			}
    			case OK_RETRY:
    			default:
    			{
    				int expirationMins = s_wizard != null ? s_wizard.getLicensedExpirationMins() : 5;
    				String msg = s_wizard.getString(R.string.MsgLicenseNotice);
    				msg = msg.replace("[000]", "" + expirationMins);

    				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
    	    		alertDialogBuilder.setTitle(s_wizard.getString(R.string.DlgTitleLicenseFailed));
    	    		alertDialogBuilder.setMessage(msg);
    	    		alertDialogBuilder.setPositiveButton(s_wizard.getString(R.string.DlgBtnRetry), new DialogInterface.OnClickListener() {
    	    			
    					@Override
    					public void onClick(DialogInterface dialog, int which) {
    						s_wizard.setLicensed(kLicenseDialogRetry);
    						//Toast.makeText(s_wizard, s_wizard.getString(R.string.ToastLicenseChecking), Toast.LENGTH_LONG).show();
    						dismiss();
    					}
    				});
    	    		alertDialogBuilder.setNegativeButton(s_wizard.getString(R.string.DlgBtnOK), new DialogInterface.OnClickListener() {
    					
    					@Override
    					public void onClick(DialogInterface dialog, int which) {
    						s_wizard.setLicensed(kLicenseDialogOK);
    						dismiss();
    					}
    				});
    	    		
    	    		setCancelable(false);
    	    		return alertDialogBuilder.create();
    			}
    		}
    	}
    }
}
