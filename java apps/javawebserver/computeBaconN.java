package ca.utoronto.utm.mcs;

import static org.neo4j.driver.v1.Values.parameters;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Map;

import org.json.JSONException;
import org.json.JSONObject;
import org.neo4j.driver.v1.Driver;
import org.neo4j.driver.v1.Record;
import org.neo4j.driver.v1.Session;
import org.neo4j.driver.v1.StatementResult;
import org.neo4j.driver.v1.Transaction;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

public class computeBaconN implements HttpHandler{
	private static Actor actor;
	Driver driver;
	public computeBaconN (Actor ac, Driver d) {
		actor = ac;
		driver=d;
	}
	@Override
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
	public void handleGet(HttpExchange r) throws IOException, JSONException { 
		String body =Utils.convert(r.getRequestBody());
		String responseResult="";
		JSONObject deserialized = new JSONObject(body);
		int response=200;
		String actorId =actor.getId();
		if (deserialized.has("actorId")) {
			actorId=deserialized.getString("actorId");
		}
		else {
			response=400;
		}
		
		actor.setId(actorId);
		StatementResult result;
		
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
				result=tx.run("MATCH p=shortestPath((bacon:Actor {name:\"Kevin Bacon\"})-[*]-(meg:Actor {id:{x}}))RETURN length(p) As BaconNumber"
						,parameters("x",actorId));
				tx.success();

		if(result.hasNext()) {
			Map record = result.next().asMap();
			responseResult+="{\n\t\"baconNumber\": "+"\""+ (Integer.valueOf(record.get("BaconNumber").toString())/2)+ "\"";
			while(result.hasNext()) {
				record=result.next().asMap();
				responseResult+= ",\n\t\t\""+record.get("movie")+"\"";
			}
			responseResult+="\n}\n";
			r.sendResponseHeaders(response,responseResult.length());
	        OutputStream os = r.getResponseBody();
	        os.write(responseResult.getBytes());
	        os.close();
			
		}
		else {
			response=400;
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
			
			}
			
		
		
		
		if (response !=200) {
			r.sendResponseHeaders(response,-1);
		}


	}
}
