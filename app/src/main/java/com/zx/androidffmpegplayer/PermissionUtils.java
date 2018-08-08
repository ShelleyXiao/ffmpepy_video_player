/*
 *
 * PermissionUtils.java
 *
 * Created by Wuwang on 2016/11/14
 * Copyright © 2016年 深圳哎吖科技. All rights reserved.
 */
package com.zx.androidffmpegplayer;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.v4.app.ActivityCompat;
import android.util.Log;

/**
 * Description:
 */
public class PermissionUtils {

    public static void askPermission(Activity context, String[] permissions, int req, Runnable
            runnable) {
        Log.i("tag", "dddddddddddddddd");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            int result = ActivityCompat.checkSelfPermission(context, permissions[0]);
            if (result == PackageManager.PERMISSION_GRANTED) {
                runnable.run();
                Log.i("tag", "dddddddddddddddddddeee");
            } else {
                Log.i("tag", "dddddddsssddddddddddddeee");
                ActivityCompat.requestPermissions(context, permissions, req);
            }
        } else {
            runnable.run();
        }
    }

    public static void onRequestPermissionsResult(boolean isReq, int[] grantResults, Runnable
            okRun, Runnable deniRun) {
        if (isReq) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                okRun.run();
            } else {
                deniRun.run();
            }
        }
    }

}
