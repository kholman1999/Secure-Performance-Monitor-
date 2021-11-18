import java.net.*;


import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SPMServer {
    public static void main(String[] args) throws Exception {
        int port= 5000;
        int maxClient = 1;
        

        ServerSocket welcomeSocket = new ServerSocket(port);
        ExecutorService executor = Executors.newFixedThreadPool(maxClient);

        while(true){

            Socket clientSocket = welcomeSocket.accept();
            Transmit tm = new Transmit(clientSocket);

            executor.execute(tm);
        }
    }
}
