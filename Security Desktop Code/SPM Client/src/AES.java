import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
//import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;


/**
 * AES Block to encrypt plaintext data from sensors and store encrypted 
 *
 * @author: Kyle Holman
 * @version: 1.0 3/22/2021
 */
public class AES
{
    public static void main(String[] args) throws Exception
    {
        if (true){
            
        String plainText = "sometxt";
        //System.out.println(plainText);
        SecretKey secKey = getSecretKey();
        String textKey= Base64.getEncoder().encodeToString(secKey.getEncoded());
        System.out.println(textKey);
        byte[] encryptedData = encryptData(plainText, secKey);
        String enc= Base64.getEncoder().encodeToString(encryptedData);
        
        System.out.println(enc);
        System.out.println("\n\n");
        //String dec = decryptData(encryptedData, secKey);
        //System.out.println(dec + "\r\n\n");
    }
       
    /*else {
     String encryptedData=args[1];
     byte[] decodedKey=Base64.getDecoder().decode(args[2]);
     byte[] decodedEncryptedData=Base64.getDecoder().decode(encryptedData);
     SecretKey key = new SecretKeySpec(decodedKey,0,decodedKey.length,"AES");
     String dec= decryptData(decodedEncryptedData,key);
     System.out.println(dec);
     System.out.println("\n\n");
     
    }
      */  
        //add fucntions using RSA functionality to encrypt secKey using pubKey and remove plaintext secKey
        
    }


    public static SecretKey getSecretKey()throws Exception 
    {
       KeyGenerator generator = KeyGenerator.getInstance("AES");
       generator.init(128); 
       SecretKey secKey = generator.generateKey();
       return secKey;
    }
    
    public static byte[] encryptData(String plaintext, SecretKey secKey) throws Exception
    {
     Cipher aesCipher = Cipher.getInstance("AES");
     aesCipher.init (Cipher.ENCRYPT_MODE,secKey);
     byte[] encryptedData = aesCipher.doFinal(plaintext.getBytes());
     return encryptedData;
    }
    
    public String decryptData(byte[] encryptedData, SecretKey secKey) throws Exception
    {
     Cipher aesCipher = Cipher.getInstance("AES");
     aesCipher.init(Cipher.DECRYPT_MODE,secKey);
     byte[] bytePlaintext=aesCipher.doFinal(encryptedData);
     return new String(bytePlaintext);
    }
}