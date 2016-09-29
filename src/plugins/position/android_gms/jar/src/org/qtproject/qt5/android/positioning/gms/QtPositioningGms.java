
package org.qtproject.qt5.android.positioning.gms;

import android.app.Activity;
import com.google.android.gms.common.GoogleApiAvailability;

public class QtPositioningGms implements ConnectionCallbacks, OnConnectionFailedListener
{
    private static final String TAG = "QtPositioningGms";

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

