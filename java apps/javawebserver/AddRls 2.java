package ca.utoronto.utm.mcs;

import static org.neo4j.driver.v1.Values.parameters;

import java.io.IOException;

import org.json.JSONException;
import org.json.JSONObject;
import org.neo4j.driver.v1.Driver;
import org.neo4j.driver.v1.Session;
import org.neo4j.driver.v1.StatementResult;
import org.neo4j.driver.v1.Transaction;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

public class AddRls implements HttpHandler{
	private static Movie movie;
	private static Actor actor;
	Driver driver;
	public AddRls (Movie mv,Actor ac, Driver d) {
		movie=mv;
		actor = ac;
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
		String actorId =actor.getId();
		String movieId=movie.getId();
		if (deserialized.has("actorId")) {
			actorId=deserialized.getString("actorId");
		}
		else {
			response=400;
		}
		if (deserialized.has("movieId")) {
			movieId=deserialized.getString("movieId");
		
		}
		else {
			response=400;
		}
		
		actor.setId(actorId);
		movie.setId(movieId);
		StatementResult result;
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
				result=tx.run("MATCH (a:Actor),(m:Movie) WHERE a.id={x} AND m.id={y} CREATE UNIQUE (a)-[r:ACTED_IN]->(m) RETURN r",parameters("x",actorId,"y",movieId));
				tx.success();

		if (result.hasNext()) {
			response=200;
		}
		else {
			response=404;
		}
	}
	catch(Exception e) {
		  response =500;
		  e.printStackTrace();
		}
	}
		catch(Exception e) {
	 response =500;
	 e.printStackTrace();
	  //  Block of code to handle errors
	}
	if (response !=200) {
		r.sendResponseHeaders(response, -1);
	}
	}
}
