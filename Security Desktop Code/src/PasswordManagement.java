import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.security.SecureRandom;
import javax.crypto.spec.SecretKeySpec;
import java.io.*;
import java.math.BigInteger;
import java.util.Scanner;





/**
 * Class defines functions used for creation, protection, 
 * and validation of user passwords.
 * 
 * Uses PBKDF2 Hash 
 * 
 * Hash(pass)=AES key for decrypting data - not saved
 * Hash(Hash(pass))= password validation - saved
 */
public class PasswordManagement
{
    private static int iterations = 10000;
    private static int keyLength = 512;
    private static SecureRandom random = new SecureRandom();
    static BigInteger mod= new BigInteger("10283216039871810935867070308763590033267924706279480036805479708407577958672010439491502023522562278580887361154108790868660131671345775095268853731990497");

    public static byte [] getNewSalt(){
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    public static String saltString(byte[] salt){
        String saltString = Base64.getEncoder().encodeToString(salt);
        return saltString;
    }

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

    public static String RSADecrypt(String encData,String pk){
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

    public static void createNewUser(String UserID, String NewPass, String SecKey) throws IOException
    {   String fname=UserID + ".txt";
        File outFile = new File (fname);
        outFile.createNewFile();
        FileOutputStream out = new FileOutputStream(outFile,false);
        BufferedWriter bw =  new BufferedWriter(new OutputStreamWriter(out));
        byte[] salt = getNewSalt();
        String saltString=saltString(salt);
        String hash=doubleHash(NewPass,salt,iterations,keyLength);
        String singleHash= hashPassword(NewPass, salt, iterations, keyLength);
        String AESkey= AESstringKey(singleHash.getBytes());
        byte[] decodedKeyBytes=Base64.getDecoder().decode(AESkey);
        SecretKey key = new SecretKeySpec(decodedKeyBytes,0,decodedKeyBytes.length,"AES");
        try{
            byte[] encData = AES.encryptData(SecKey, key);
            String RSAString = Base64.getEncoder().encodeToString(encData);
            bw.write(hash);
            bw.newLine();
            bw.write(saltString);
            bw.newLine();
            bw.write(RSAString);
            bw.close();
        }
        catch (Exception e){
            throw new RuntimeException(e);
        }
    }   
    
    public static void decryptData(String UserID, String pass) throws Exception{
        String fname = UserID+".txt";
        File loginFile = new File(fname);
        FileInputStream in = new FileInputStream(loginFile);
        BufferedReader br = new BufferedReader(new InputStreamReader(in));
        String hash2 = br.readLine();
        String salt=br.readLine();
        byte [] saltb = saltByte(salt);
        if(validate(pass,hash2,saltb,iterations,keyLength)){
            String encRSAkey= br.readLine();
            br.close();
            AES AES1 = new AES();
            String singleHash= hashPassword(pass, saltb, iterations, keyLength);
            String AESkey= AESstringKey(singleHash.getBytes());
            byte[] decodedKeyBytes=Base64.getDecoder().decode(AESkey);
            SecretKey key = new SecretKeySpec(decodedKeyBytes,0,decodedKeyBytes.length,"AES");
            String RSAkey = AES1.decryptData(Base64.getDecoder().decode(encRSAkey), key);
            
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
                    String decLine = RSADecrypt(encLine,RSAkey);
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
        else{
            System.out.println("Incorrect Password");
        }


    }

    public static void main(String[] args){
        Scanner scan =new Scanner(System.in);
        System.out.println("Enter Username: ");
        String UsrName=scan.nextLine();
        File fname= new File(UsrName+".txt");
        if(fname.exists()){
            System.out.println("Enter Password: ");
            String pass = scan.nextLine();
            try {
                decryptData(UsrName,pass);
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

        }
        else{
            System.out.println("Account not found, press enter to exit or type password to create new account");
            String pass = scan.nextLine();
            if(!(pass.equals(""))){
                System.out.println("Enter RSA private key");
                String key = scan.nextLine();
                try {
                    createNewUser(UsrName,pass,key);
                    System.out.println("Account Created. Log in to Access Data");
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    System.out.println("unable to create account");
                }
            }
        }
        scan.close();

/*
        String userID = "kholmanTest";
        String Password = "testPass";
        String key= "6616332800847754323077262291685100564040602717993773977169417176727356361005891109222083628857046702461025280501144890524091810012785071487741966801224321";
        try {
            createNewUser(userID,Password,key);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        try {
            decryptData("kholmanTest", "testPass");
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
            String input1 = args[0];
            String input2= args[1];
            String testPhrase= args[2];
            
            byte[] salt = getNewSalt();
            String saltString = saltString(salt);
            String doubleHash1= doubleHash(input1, salt, iterations, keyLength);
            boolean validate = validate(input2,doubleHash1,salt,iterations,keyLength);
            
            if (validate){
                AES AES1 = new AES();
                String singleHash= hashPassword(input2, salt, iterations, keyLength);
                String AESkey= AESstringKey(singleHash.getBytes());
                System.out.println(AESkey);
                byte[] decodedKeyBytes=Base64.getDecoder().decode(AESkey);
                SecretKey key = new SecretKeySpec(decodedKeyBytes,0,decodedKeyBytes.length,"AES");
                try{
                byte[] encData = AES1.encryptData(testPhrase, key);
                String decData = AES1.decryptData(encData,key);
                System.out.println(decData);
                }
                catch (Exception e){
                    throw new RuntimeException(e);
                }
                
            }
            else {
              System.out.println("could not validate pass");  
            }
            
            return validate;
*/
        }

}
