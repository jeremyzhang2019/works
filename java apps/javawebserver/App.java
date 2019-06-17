package ca.utoronto.utm.mcs;
import org.neo4j.driver.v1.*;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.Enumeration;
import java.util.Properties;

import org.neo4j.driver.v1.AuthTokens;
import org.neo4j.driver.v1.GraphDatabase;

import com.sun.net.httpserver.HttpContext;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.net.InetSocketAddress;
import com.sun.net.httpserver.HttpServer;

public class App 
{	
	static String uniqueActorQ="CREATE CONSTRAINT ON (n:Actor) ASSERT n.id IS UNIQUE";
	static String uniqueMovieQ="CREATE CONSTRAINT ON (n:Movie) ASSERT n.id IS UNIQUE";
	static String uri="bolt://localhost:7687";
	private static String ppFileName="src/main/java/ca/utoronto/utm/mcs/application.properties";
	private static String username;
	private static String password;
    static int PORT = 8080;
    @SuppressWarnings("restriction")
	public static void main(String[] args) throws IOException
    {	
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", PORT), 0);
       //  driver;
        try {
        	ini();
        	Driver driver = GraphDatabase.driver(uri, AuthTokens.basic(username, password));
        makeUnique(driver);
        Actor ac = new Actor();
        Movie mv=new Movie();
        server.createContext("/api/v1/addActor",new AddActor(ac,driver));
        server.createContext("/api/v1/addMovie",new AddMovie(mv,driver));
        server.createContext("/api/v1/addRelationship",new AddRls(mv,ac,driver));
        server.createContext("/api/v1/getActor",new getActor(ac,driver));
        server.createContext("/api/v1/getMovie",new getMovie(mv,driver));
        server.createContext("/api/v1/hasRelationship",new hasRls(ac,mv,driver));
        server.createContext("/api/v1/computeBaconNumber",new computeBaconN(ac,driver));
        server.createContext("/api/v1/computeBaconPath",new computeBaconP(ac,driver));
        server.start();
        System.out.printf("Server started on port %d...\n", PORT);
        }
        catch (Exception e) {
        	e.printStackTrace();
        }

    }
    private static void makeUnique(Driver driver) {
		try (Session session=driver.session()){
			try(Transaction tx =session.beginTransaction()){
				tx.run(uniqueActorQ);
				tx.run(uniqueMovieQ);
				tx.success();
			}
		}
    }
    private static void ini() throws FileNotFoundException, IOException {
    	Properties pps = new Properties();
    	pps.load(new FileInputStream(ppFileName));
    	username=pps.getProperty("username");
    	password=pps.getProperty("password");;
    }
	
}
