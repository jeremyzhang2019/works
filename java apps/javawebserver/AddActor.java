package ca.utoronto.utm.mcs;
import org.neo4j.driver.v1.*;

import java.io.IOException;
import java.io.OutputStream;

import org.json.*;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

import static org.neo4j.driver.v1.Values.parameters;

public class AddActor implements HttpHandler{
	private static Actor actor;
	Driver driver;
	public AddActor(Actor mem, Driver d) {
		actor=mem;
		driver=d;
	}
	@Override
	public void handle(HttpExchange r) {
	       try {
	    	   if (r.getRequestMethod().equals("PUT")) {
	                handlePut(r);
	            }
	            else {
	            	r.sendResponseHeaders(400, -1);
	            }
	        } catch (Exception e) {
	            e.printStackTrace();
	        }
	}
	public void handlePut(HttpExchange r) throws IOException, JSONException { 
		String body =Utils.convert(r.getRequestBody());
		JSONObject deserialized = new JSONObject(body);
		int response=200;
		String name =actor.getName();
		String actorId=actor.getId();
		if (deserialized.has("name")) {
			name=deserialized.getString("name");
		}
		else {
			response=400;
		}
		if (deserialized.has("actorId")) {
			actorId=deserialized.getString("actorId");
		
		}
		else {
			response=400;
		}
		
		actor.setName(name);
		actor.setId(actorId);
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
				tx.run("MERGE(a:Actor {name:{x},id:{y}})",parameters("x",name,"y",actorId));
				tx.success();
			}

		catch(Exception e) {
			  response =500;
			  e.printStackTrace();
			}
		}	
		catch(Exception e) {
			 response =500;
			 e.printStackTrace();
			}

		r.sendResponseHeaders(response, -1);
	}


}
