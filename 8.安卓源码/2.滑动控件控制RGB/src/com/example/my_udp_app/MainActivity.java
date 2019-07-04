package com.example.my_udp_app;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import com.example.my_udp_app.R.id;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class MainActivity extends Activity {
	
	private EditText IP_address;
	private EditText IP_Port;
	private Button connect;
	private Button disconnect;
	
	private SeekBar redbar;
	private SeekBar bluebar;
	private SeekBar greenbar;
	
	private Thread newThread; //声明一个子线程  
	DatagramSocket socket = null;
    InetAddress serverAddress = null;
    
    String IP;
    int port;
    
    void APP_INIT()
    {
    	IP_address = (EditText)findViewById(R.id.editText1);
    	IP_Port = (EditText)findViewById(R.id.editText2);
		
    	connect=(Button)findViewById(R.id.button1);
    	disconnect=(Button)findViewById(R.id.button2);
    	disconnect.setEnabled(false);
    	
    	redbar=(SeekBar) findViewById(R.id.seekBar1);
    	redbar.setEnabled(false);
    	bluebar=(SeekBar) findViewById(R.id.SeekBar01);
    	bluebar.setEnabled(false);
    	greenbar=(SeekBar) findViewById(R.id.SeekBar02);
    	greenbar.setEnabled(false);
    }
    
    void Star_App()
    {
    	IP_address.setEnabled(false);
    	IP_Port.setEnabled(false);
		
    	disconnect.setEnabled(true);
    	connect.setEnabled(false);
    	
    	greenbar.setEnabled(true);
    	bluebar.setEnabled(true);
    	redbar.setEnabled(true);
    	
		redbar.setOnSeekBarChangeListener(new redListener());
		bluebar.setOnSeekBarChangeListener(new blueListener());
		greenbar.setOnSeekBarChangeListener(new greenListener());
    }
    
    void Close_App()
    {
    	IP_address.setEnabled(true);
    	IP_Port.setEnabled(true);
		
    	disconnect.setEnabled(false);
    	connect.setEnabled(true);
    	
    	//greenbar.setEnabled(false);
    	//bluebar.setEnabled(false);
    	//redbar.setEnabled(false);
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        APP_INIT();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    public void unconnet(View v)
    {
    	Close_App();
    	newThread.interrupt();
    }
    
    public void connet(View v)
    {
    	Star_App();
    	
    	if(IP_address.getText().toString().trim()!=null && IP_Port.getText().toString().trim()!=null)
    	{
    	  IP=IP_address.getText().toString().trim();
    	  port=Integer.parseInt(IP_Port.getText().toString().trim());
    	}else {
    		IP= "192.168.155.2";
    		port=8033;
    	}
    	try{
            socket = new DatagramSocket(port);
            serverAddress = InetAddress.getByName(IP);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }
    
    
    void send_udp(final String string)
    {
    	newThread = new Thread(new Runnable() {   
    	    @Override   
    	    public void run() {   
    	    	try{
    	               String sendData =string;
    	               byte data[] = sendData.getBytes();
    	               DatagramPacket packet = new DatagramPacket(data, data.length, serverAddress, port);
    	               socket.send(packet);
    	        } catch(Exception e) {
    	            e.printStackTrace();
    	        }//这里写入子线程需要做的工作   
    	    	newThread.interrupt();
    	    }  
    	});   
    	newThread.start(); //启动线程  
    }
    
    private class redListener implements OnSeekBarChangeListener{
    	@Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, final int progress,
                boolean fromUser) {
        	send_udp("R:"+Integer.toString(progress)+"%");
        }
    }
    
    private class blueListener implements OnSeekBarChangeListener{
    	@Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, final int progress,
                boolean fromUser) {
        	send_udp("B:"+Integer.toString(progress)+"%");
        }
    }
    
    private class greenListener implements OnSeekBarChangeListener{
    	@Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, final int progress,
                boolean fromUser) {
        	send_udp("G:"+Integer.toString(progress)+"%");
        }
    }
}
