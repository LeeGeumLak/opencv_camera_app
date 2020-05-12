package com.example.lglcamera.activity;

import android.app.ListActivity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.example.lglcamera.adapter.HistoryAdapter;
import com.example.lglcamera.data.HistoryItem;
import com.example.lglcamera.utils.Constants;

import com.pubnub.api.Callback;
import com.pubnub.api.Pubnub;
import com.pubnub.api.PubnubError;
import com.pubnub.api.PubnubException;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

import com.example.lglcamera.R;

public class FaceTalkActivity extends ListActivity {
    private SharedPreferences sharedPreferences;
    private String username;
    private String stdByChannel;
    private Pubnub pubNub;

    private ListView historyList;
    private HistoryAdapter historyAdapter;
    private EditText callNumET;
    private TextView usernameTV;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_face_talk);

        this.sharedPreferences = getSharedPreferences(Constants.SHARED_PREFS, MODE_PRIVATE);
        if (!this.sharedPreferences.contains(Constants.USER_NAME)){
            Intent intent = new Intent(this, LoginActivity.class);
            startActivity(intent);
            finish();
            return;
        }
        this.username     = this.sharedPreferences.getString(Constants.USER_NAME, "");
        this.stdByChannel = this.username + Constants.STDBY_SUFFIX;

        this.historyList = getListView();
        this.callNumET   = (EditText) findViewById(R.id.call_num);
        this.usernameTV  = (TextView) findViewById(R.id.main_username);

        this.usernameTV.setText(this.username);
        initPubNub();

        this.historyAdapter = new HistoryAdapter(this, new ArrayList<HistoryItem>(), this.pubNub);
        this.historyList.setAdapter(this.historyAdapter);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_face_talk, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        switch(id){
            case R.id.action_settings:
                return true;
            case R.id.action_sign_out:
                signOut();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStop() {
        super.onStop();
        if(this.pubNub!=null){
            this.pubNub.unsubscribeAll();
        }
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        if(this.pubNub==null){
            initPubNub();
        } else {
            subscribeStdBy();
        }
    }

    /**
     * Subscribe to standby channel so that it doesn't interfere with the WebRTC Signaling.
     */
    public void initPubNub(){
        this.pubNub  = new Pubnub(Constants.PUB_KEY, Constants.SUB_KEY);
        this.pubNub.setUUID(this.username);
        subscribeStdBy();
    }

    /**
     * Subscribe to standby channel
     */
    private void subscribeStdBy(){
        try {
            this.pubNub.subscribe(this.stdByChannel, new Callback() {
                @Override
                public void successCallback(String channel, Object message) {
                    Log.d("MA-iPN", "MESSAGE: " + message.toString());
                    if (!(message instanceof JSONObject)) return; // Ignore if not JSONObject
                    JSONObject jsonMsg = (JSONObject) message;
                    try {
                        if (!jsonMsg.has(Constants.JSON_CALL_USER)) return;     //Ignore Signaling messages.
                        String user = jsonMsg.getString(Constants.JSON_CALL_USER);
                        dispatchIncomingCall(user);
                    } catch (JSONException e){
                        e.printStackTrace();
                    }
                }

                @Override
                public void connectCallback(String channel, Object message) {
                    Log.d("MA-iPN", "CONNECTED: " + message.toString());
                    setUserStatus(Constants.STATUS_AVAILABLE);
                }

                @Override
                public void errorCallback(String channel, PubnubError error) {
                    Log.d("MA-iPN","ERROR: " + error.toString());
                }
            });
        } catch (PubnubException e){
            Log.d("HERE","HEREEEE");
            e.printStackTrace();
        }
    }

    /**
     * Take the user to a video screen. USER_NAME is a required field.
     * @param view button that is clicked to trigger toVideo
     */
    public void makeCall(View view){
        String callNum = callNumET.getText().toString();
        if (callNum.isEmpty() || callNum.equals(this.username)){
            showToast("Enter a valid user ID to call.");
            return;
        }
        dispatchCall(callNum);
    }

    /**TODO: Debate who calls who. Should one be on standby? Or use State API for busy/available
     * Check that user is online. If they are, dispatch the call by publishing to their standby
     *   channel. If the publish was successful, then change activities over to the video chat.
     * The called user will then have the option to accept of decline the call. If they accept,
     *   they will be brought to the video chat activity as well, to connect video/audio. If
     *   they decline, a hangup will be issued, and the VideoChat adapter's onHangup callback will
     *   be invoked.
     * @param callNum Number to publish a call to.
     */
    public void dispatchCall(final String callNum){
        final String callNumStdBy = callNum + Constants.STDBY_SUFFIX;
        this.pubNub.hereNow(callNumStdBy, new Callback() {
            @Override
            public void successCallback(String channel, Object message) {
                Log.d("MA-dC", "HERE_NOW: " +" CH - " + callNumStdBy + " " + message.toString());
                try {
                    int occupancy = ((JSONObject) message).getInt(Constants.JSON_OCCUPANCY);
                    if (occupancy == 0) {
                        showToast("User is not online!");
                        return;
                    }
                    JSONObject jsonCall = new JSONObject();
                    jsonCall.put(Constants.JSON_CALL_USER, username);
                    jsonCall.put(Constants.JSON_CALL_TIME, System.currentTimeMillis());
                    pubNub.publish(callNumStdBy, jsonCall, new Callback() {
                        @Override
                        public void successCallback(String channel, Object message) {
                            Log.d("MA-dC", "SUCCESS: " + message.toString());
                            Intent intent = new Intent(FaceTalkActivity.this, VideoChatActivity.class);
                            intent.putExtra(Constants.USER_NAME, username);
                            intent.putExtra(Constants.CALL_USER, callNum);  // Only accept from this number?
                            startActivity(intent);
                        }
                    });
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Handle incoming calls. TODO: Implement an accept/reject functionality.
     * @param userId
     */
    private void dispatchIncomingCall(String userId){
        showToast("Call from: " + userId);
        Intent intent = new Intent(FaceTalkActivity.this, IncomingCallActivity.class);
        intent.putExtra(Constants.USER_NAME, username);
        intent.putExtra(Constants.CALL_USER, userId);
        startActivity(intent);
    }

    private void setUserStatus(String status){
        try {
            JSONObject state = new JSONObject();
            state.put(Constants.JSON_STATUS, status);
            this.pubNub.setState(this.stdByChannel, this.username, state, new Callback() {
                @Override
                public void successCallback(String channel, Object message) {
                    Log.d("MA-sUS","State Set: " + message.toString());
                }
            });
        } catch (JSONException e){
            e.printStackTrace();
        }
    }

    private void getUserStatus(String userId){
        String stdByUser = userId + Constants.STDBY_SUFFIX;
        this.pubNub.getState(stdByUser, userId, new Callback() {
            @Override
            public void successCallback(String channel, Object message) {
                Log.d("MA-gUS", "User Status: " + message.toString());
            }
        });
    }

    /**
     * Ensures that toast is run on the UI thread.
     * @param message
     */
    private void showToast(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(FaceTalkActivity.this, message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * Log out, remove username from SharedPreferences, unsubscribe from PubNub, and send user back
     *   to the LoginActivity
     */
    public void signOut(){
        this.pubNub.unsubscribeAll();
        SharedPreferences.Editor edit = this.sharedPreferences.edit();
        edit.remove(Constants.USER_NAME);
        edit.apply();
        Intent intent = new Intent(this, LoginActivity.class);
        intent.putExtra("oldUsername", this.username);
        startActivity(intent);
    }
}