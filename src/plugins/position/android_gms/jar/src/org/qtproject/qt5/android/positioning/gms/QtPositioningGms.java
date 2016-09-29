
package org.qtproject.qt5.android.positioning.gms;

import android.app.Activity;
import com.google.android.gms.common.GoogleApiAvailability;

public class QtPositioningGms implements ConnectionCallbacks, OnConnectionFailedListener
{
    private static final String TAG = "QtPositioningGms";

    /* The following values must match the corresponding error enums in the Qt API*/
    public static final int QT_ACCESS_ERROR = 0;
    public static final int QT_CLOSED_ERROR = 1;
    public static final int QT_POSITION_UNKNOWN_SOURCE_ERROR = 2;
    public static final int QT_POSITION_NO_ERROR = 3;

    static public boolean isAvailable(final Activity activity) {
        try {
            final GoogleApiAvailability apiAvailability = GoogleApiAvailability.getInstance();
            final int errorCode = apiAvailability.isGooglePlayServicesAvailable(activity);
            return ConnectionResult.SUCCESS == errorCode;
        } catch(Exception e) {
            Log.e(TAG, "Failed to check google api availability: ", e);
        }

        return false;
    }

}

