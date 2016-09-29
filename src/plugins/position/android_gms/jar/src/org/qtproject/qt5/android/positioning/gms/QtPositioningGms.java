    

package org.qtproject.qt5.android.positioning.gms;

import android.app.Activity;
import android.os.Looper;
import com.google.android.gms.common.GoogleApiAvailability;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.common.api.GoogleApiClient.ConnectionCallbacks;
import com.google.android.gms.common.api.GoogleApiClient.OnConnectionFailedListener;

public class QtPositioningGms implements ConnectionCallbacks, OnConnectionFailedListener
{
    private static final String TAG = "QtPositioningGms";

    /* The following values must match the corresponding error enums in the Qt API*/
    public static final int QT_ACCESS_ERROR = 0;
    public static final int QT_CLOSED_ERROR = 1;
    public static final int QT_POSITION_UNKNOWN_SOURCE_ERROR = 2;
    public static final int QT_POSITION_NO_ERROR = 3;


    private GoogleApiClient mGoogleApiClient = new GoogleApiClient.Builder(getActivity())
        .addConnectionCallbacks(this)
        .addOnConnectionFailedListener(this)
        .addApi(LocationServices.API)
        .build();

    private Looper mlocationUpdatesLooper = null;
    final private Thread mlocationUpdatesThread = new Thread() {
        public void run() {
            Looper.prepare();
            mlocationUpdatesLooper = Looper.myLooper();
            Looper.loop();
        }
    };


    public void activate(boolean enable)
    {
        if (null != mGoogleApiClient) {
            if (enable) {
                mGoogleApiClient.connect();
            } else {
                mGoogleApiClient.disconnect();
            }           
        }
    }

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

