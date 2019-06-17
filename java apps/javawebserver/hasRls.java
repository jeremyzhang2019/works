package ca.utoronto.utm.mcs;
import static org.neo4j.driver.v1.Values.parameters;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Map;

import org.json.JSONException;
import org.json.JSONObject;
import org.neo4j.driver.v1.Driver;
import org.neo4j.driver.v1.Session;
import org.neo4j.driver.v1.StatementResult;
import org.neo4j.driver.v1.Transaction;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

public class hasRls implements HttpHandler{
	private static Actor actor;
	private static Movie mv;
	Driver driver;
	public hasRls (Actor ac,Movie m, Driver d) {
		actor = ac;
		driver=d;
		mv = m;
	}
	@Override

// check which handle function is needed in this case only GET is needed
	public void handle(HttpExchange r) {
	       try {
	            if (r.getRequestMethod().equals("GET")) {
	                handleGet(r);
	            }
	            else {
	            	r.sendResponseHeaders(400, -1);
	            }
	        } catch (Exception e) {
	            e.printStackTrace();
	        }
	}
	
	
// start the handle function 
	public void handleGet(HttpExchange r) throws IOException, JSONException { 
		String body =Utils.convert(r.getRequestBody());
		String responseResult="";
		JSONObject deserialized = new JSONObject(body);
		int response=200;
		
		
// get actor id from the input 		
		String actorId =actor.getId();
		if (deserialized.has("actorId")) {
			actorId=deserialized.getString("actorId");
		}
		else {
			response=400;
		}
		actor.setId(actorId);
		
//get the movieId from the input		
		String movieId = mv.getId();
		if (deserialized.has("movieId")) {
			movieId=deserialized.getString("movieId");
		}
		else {
			response=400;
		}
		mv.setId(movieId);

//now movie and actor has store their ID inside 
		

		
		
		

		StatementResult result;
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
//###MATCH(a:Actor{ id: {x}}) -[:ACTED_IN]->(m) return a.id As actorId,a.name As name,m.id As movie",parameters("x",actorId)
//###MATCH  (a:Actor {actorId: {x}}), (b:Movie {movieId: {y}}) RETURN EXISTS((a)-[:ACTED_IN]-(b))",parameters("x",actorId,"y",movieId)
				result=tx.run("RETURN EXISTS((:Actor{id:{x}})-[:ACTED_IN]->(:Movie{id:{y}}))",parameters("x",actorId,"y",movieId));
				tx.success();
			}
		}

		
			responseResult+="{\n\t\"actorId\":\" "+actorId+"\",\n\t\"movieId\":\""+movieId+"\",\n\t\"hasRelationship\":\""+result.single().get(0).asBoolean()+"\"\t\n}\n";

			r.sendResponseHeaders(response,responseResult.length());
	        OutputStream os = r.getResponseBody();
	        os.write(responseResult.getBytes());
	        os.close();
			
//		}
//		else {
//			response=400;
//		}
		if (response !=200) {
			r.sendResponseHeaders(response,-1);
		}


	}
}
