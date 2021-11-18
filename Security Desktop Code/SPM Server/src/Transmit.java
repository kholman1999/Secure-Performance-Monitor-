import java.io.*;
import java.net.*;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.security.SecureRandom;
import javax.crypto.spec.SecretKeySpec;
import java.math.BigInteger;


public class Transmit implements Runnable{
    private Socket client;
    
    
    private static int iterations = 10000;
    private static int keyLength = 512;
    private static SecureRandom random = new SecureRandom();
    
    


    public static byte [] getNewSalt(){
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    public static String saltString(byte[] salt){
        String saltString = Base64.getEncoder().encodeToString(salt);
        return saltString;
    }

    
    
    public static String hashPassword( final String password, final byte[] salt,final int iterations, final int keyLength)
    {
      try{  
      char[] charPass=password.toCharArray();  
      SecretKeyFactory skf= SecretKeyFactory.getInstance("PBKDF2WithHmacSHA512");
      PBEKeySpec spec= new PBEKeySpec(charPass,salt,iterations,keyLength);
      SecretKey secKey = skf.generateSecret(spec);
      String res = Base64.getEncoder().encodeToString(secKey.getEncoded());
      return res;
      
    }
    
    catch (NoSuchAlgorithmException | InvalidKeySpecException e){
        throw new RuntimeException(e);
    }
    }
    
    public static String AESstringKey(byte[] byteKey){
        byte[] AESkey = new byte[16];
        
        for (int i=0; i<16; i++){
         AESkey[i]=byteKey[i];   
        }
        String key = Base64.getEncoder().encodeToString(AESkey);
        return key;
        
    }
    
    public static String doubleHash(String Pass, byte[] salt, int iterations, int keyLength){
        
        String round1 = hashPassword(Pass, salt, iterations, keyLength);
        String round2 = hashPassword(round1, salt, iterations, keyLength);
        //System.out.println(round2);
        
        return round2;
        
    }
    
    public static boolean validate(String pass, String doubleHash1, byte[] salt, int iterations, int keyLength){
        String EncInputPass =  doubleHash(pass, salt, iterations, keyLength);
        if (doubleHash1.equals(EncInputPass)){
            return true;
        }
        else return false;
    }

    

    public static void createNewUser(String UserID, String NewPass) throws IOException
    {   String fname=UserID + ".txt";
        String pubName = "Public"+UserID+".txt";
        File outFile = new File (fname);
        File pubFile = new File (pubName);
        outFile.createNewFile();
        pubFile.createNewFile();
        FileOutputStream out = new FileOutputStream(outFile,false);
        BufferedWriter bw =  new BufferedWriter(new OutputStreamWriter(out));
        FileOutputStream Pout = new FileOutputStream(pubFile,false);
        BufferedWriter Pbw =  new BufferedWriter(new OutputStreamWriter(Pout));

        byte[] salt = getNewSalt();
        String saltString=saltString(salt);
        String hash=doubleHash(NewPass,salt,iterations,keyLength);
        String singleHash= hashPassword(NewPass, salt, iterations, keyLength);
        String AESkey= AESstringKey(singleHash.getBytes());
        byte[] decodedKeyBytes=Base64.getDecoder().decode(AESkey);
        SecretKey key = new SecretKeySpec(decodedKeyBytes,0,decodedKeyBytes.length,"AES");
        try{
            BigInteger publicKey=new BigInteger("4095");
            BigInteger privateKey=new BigInteger("0");
            BigInteger mod=new BigInteger("0");
            while(true){
                 try {
                    int N=1024;
                    BigInteger one = new BigInteger("1");
                    BigInteger p= BigInteger.probablePrime(N/2, random);
                    BigInteger q= BigInteger.probablePrime(N/2,random);
                    BigInteger phi= (p.subtract(one)).multiply(q.subtract(one));
                   
                    BigInteger r= p.multiply(q);
                    mod=r;
                    privateKey= publicKey.modInverse(phi);
                   break;
                } catch (Exception e) {
                
                }
            }
            

            String pubStr = publicKey.toString();
            String modStr = mod.toString();
            String priStr = mod.toString();
            privateKey.multiply(new BigInteger("0"));

            byte[] encData = AES.encryptData(priStr, key);
            String RSAString = Base64.getEncoder().encodeToString(encData);
            bw.write(hash);
            bw.newLine();
            bw.write(saltString);
            bw.newLine();
            bw.write(RSAString);
            bw.close();
            Pbw.write(pubStr);
            Pbw.newLine();
            Pbw.write(modStr);
            Pbw.close();

        }
        catch (Exception e){
            throw new RuntimeException(e);
        }
    }   

   

    
    public Transmit(Socket Client){
        this.client=Client;
        
    }

    @Override
    public void run(){
        try{
            BufferedWriter toClient = new BufferedWriter(new OutputStreamWriter(client.getOutputStream()));
            BufferedReader fromClient = new BufferedReader(new InputStreamReader(client.getInputStream()));
            
            String userName= fromClient.readLine();
            //System.out.println(userName);
            File fname= new File(userName+".txt");
            if(fname.exists()){
                toClient.write("1"+"\n");
                toClient.flush();
                FileInputStream in = new FileInputStream(fname);
                BufferedReader br = new BufferedReader(new InputStreamReader(in));
                String hash2 = br.readLine();
                String salt=br.readLine();
                String encRSAkey=br.readLine();
                br.close();
                toClient.write(hash2+"\n");
                toClient.flush();
                toClient.write(salt+"\n");
                toClient.flush();
                toClient.write(encRSAkey+"\n");
                toClient.flush();

                File pubFile= new File("Public"+userName+".txt");
                FileInputStream in2 = new FileInputStream(pubFile);
                BufferedReader br2 = new BufferedReader(new InputStreamReader(in2));
                br2.readLine();
                String k2=br2.readLine();
                toClient.write(k2+"\n");
                toClient.flush();
                br2.close();
            }
            else{
                toClient.write("0"+"\n");
                toClient.flush();
                String pass = fromClient.readLine();
                if(!(pass.equals(""))){
                    try{
                        createNewUser(userName, pass);
                        String created="1";
                        toClient.write(created+"\n");
                        toClient.flush();
                    }
                    catch(Exception e){
                        String created="0";
                        toClient.write(created+"\n");
                        toClient.flush();
                    }
                }
            }
       
            client.close();
        }
        catch (Exception e){
            e.printStackTrace();
        }
    }

}

