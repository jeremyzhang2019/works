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

public class getMovie implements HttpHandler{
	private static Movie movie;
	Driver driver;
	public getMovie (Movie mv, Driver d) {
		movie = mv;
		driver=d;
	}
	@Override
	public void handle(HttpExchange r) {
	       try {
	            if (r.getRequestMethod().equals("GET")) {
	                handleGet(r);
	            } else {
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
		String movieId =movie.getId();
		if (deserialized.has("movieId")) {
			movieId=deserialized.getString("movieId");
		}
		else {
			response=400;
		}
		
		movie.setId(movieId);
		StatementResult result;
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
				result=tx.run("MATCH(m:Movie{ id:{x} }) <-[:ACTED_IN]-(a) return m.id As movieId,m.name As name,a.id As actor",parameters("x",movieId));
				tx.success();

		
		if(result.hasNext()) {
			Map record = result.next().asMap();
			responseResult+="{\n\t\"movieId\":\" "+movieId+"\",\n\t\"name\": \""+record.get("name")+"\",\n\t\"actor\": [\n\t\t\""+record.get("actor")+"\"";
			while(result.hasNext()) {
				record=result.next().asMap();
				responseResult+= ",\n\t\t\""+record.get("actor")+"\"";
			}
			responseResult+="\n\t]\n}\n";
			r.sendResponseHeaders(response,responseResult.length());
	        OutputStream os = r.getResponseBody();
	        os.write(responseResult.getBytes());
	        os.close();
			
		}
		else {
			response=404;
		}
		}
		catch(Exception e) {
			e.printStackTrace();
			  response =500;
			}
	}
	catch(Exception e) {
		e.printStackTrace();
		 response =500;
		  //  Block of code to handle errors
		}
		
		if (response !=200) {
			r.sendResponseHeaders(response,-1);
		}


	}
}
