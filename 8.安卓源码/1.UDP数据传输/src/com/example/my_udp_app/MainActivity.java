package com.example.my_udp_app;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	
	private EditText IP_address;
	private EditText IP_Port;
	private Button connect;
	private Button disconnect;
	
	private EditText sendstring;
	private Button senddatakey;
	

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
    	
    	sendstring=(EditText) findViewById(R.id.editText3);
    	senddatakey=(Button) findViewById(R.id.button6);
    	senddatakey.setEnabled(false);
    }
    
    void Star_App()
    {
    	IP_address.setEnabled(false);
    	IP_Port.setEnabled(false);
		
    	disconnect.setEnabled(true);
    	connect.setEnabled(false);
    	
    	senddatakey.setEnabled(true);
    }
    
    void Close_App()
    {
    	IP_address.setEnabled(true);
    	IP_Port.setEnabled(true);
		
    	disconnect.setEnabled(false);
    	connect.setEnabled(true);
    	
    	senddatakey.setEnabled(false);
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
    
    public void senddata(View v)
    {
    	newThread = new Thread(new Runnable() {   
    	    @Override   
    	    public void run() {   
    	    	try{
    	               String sendData = sendstring.getText().toString().trim();
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
}
