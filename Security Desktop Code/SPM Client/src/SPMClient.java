import java.io.BufferedReader;
import java.net.*;
import java.io.*;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import javax.crypto.spec.SecretKeySpec;
import java.math.BigInteger;
import java.util.Scanner;

public class SPMClient {
 
    private static int iterations = 10000;
    private static int keyLength = 512;
   
    
    
   

    public static byte[] saltByte(String salt){
        byte[] saltbytes = Base64.getDecoder().decode(salt);
        return saltbytes;
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

    public static String RSADecrypt(String encData,String pk, BigInteger mod){
        BigInteger privKey=new BigInteger(pk);
        BigInteger encDataNum= new BigInteger(encData);
        BigInteger intPass=encDataNum.modPow(privKey,mod);

        String table="==========1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ.  ";

        String output="";
        BigInteger zero= new BigInteger("0");
        BigInteger rem;
        BigInteger hund= new BigInteger("100");
        
        while ((intPass.compareTo(zero))==1){
            rem=intPass.mod(hund);
            intPass=intPass.subtract(rem);
            intPass=intPass.divide(hund);
            int index = rem.intValue();
            output=output+""+table.charAt(index);
        }
        return output;
    }

    

    public static void decryptData(String pass, String salt, String encRSAKey,String mod) throws Exception{
       BigInteger mod1 = new BigInteger(mod);
        byte [] saltb = saltByte(salt);
        
        AES AES1 = new AES();
        String singleHash= hashPassword(pass, saltb, iterations, keyLength);
        String AESkey= AESstringKey(singleHash.getBytes());
        byte[] decodedKeyBytes=Base64.getDecoder().decode(AESkey);
        SecretKey key = new SecretKeySpec(decodedKeyBytes,0,decodedKeyBytes.length,"AES");
        String RSAkey = AES1.decryptData(Base64.getDecoder().decode(encRSAKey), key);
            
        File encDataFile = new File("encData.txt");
        FileInputStream in2 = new FileInputStream(encDataFile);
        BufferedReader br2 = new BufferedReader(new InputStreamReader(in2));

        File decDataFile = new File("decData.txt");
        decDataFile.createNewFile();
        FileOutputStream out = new FileOutputStream(decDataFile);
        BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(out));
            
        while(br2.ready()){
                String encLine = br2.readLine();
                try {
                    String decLine = RSADecrypt(encLine,RSAkey,mod1);
                    bw.write(decLine);
                    bw.newLine();
                } catch (Exception e) {
                    //TODO: handle exception
                    System.out.println("Unable to decrypt data");
                }
                
                

        }
        bw.close();
        br2.close(); 
    }
    
    public static void main(String argv[]) throws Exception{
        InetAddress ServerIP= InetAddress.getByName("127.0.0.1");
        int ServerPort = 5000;

        Scanner scan =new Scanner(System.in);
        System.out.println("Enter Username: ");
        String UsrName=scan.nextLine();
        
        Socket ServerConnect = new Socket(ServerIP,ServerPort);
        BufferedReader fromServer = new BufferedReader(new InputStreamReader(ServerConnect.getInputStream()));
        BufferedWriter toServer=new BufferedWriter(new OutputStreamWriter(ServerConnect.getOutputStream()));
        toServer.write(UsrName+"\n");
        toServer.flush();
        String reply1= fromServer.readLine();
        if(reply1.equals("0")){
            System.out.println("Account not found, press enter to exit or type password to create new account");
            String pass = scan.nextLine();
            if(!(pass.equals(""))){
                toServer.write(pass+"\n");
                toServer.flush();
                String created = fromServer.readLine();
                if(created.equals("1")){
                    System.out.println("account created. Restart to attempt login");
                    ServerConnect.close();
                    System.exit(0);
                }
                else{
                    System.out.println("unable to create account, please try again");
                    ServerConnect.close();
                    System.exit(0);
                }
            }
            
        } 
        else{
            System.out.println("Enter Password: ");
            String pass = scan.nextLine();
            String hash2=fromServer.readLine();
            String salt=fromServer.readLine();
            byte [] saltb = saltByte(salt);

            if(validate(pass, hash2, saltb, iterations, keyLength)){
                String encKey=fromServer.readLine();
                String mod = fromServer.readLine();
                decryptData(pass,salt,encKey,mod);
                ServerConnect.close();
                System.exit(0);
            }
            else{
                System.out.println("Incorrect Password: closing");
                ServerConnect.close();
                System.exit(0);
            }

        }  
        scan.close();
        ServerConnect.close();
    }
}
