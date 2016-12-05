package com.cheekymammoth.puzzlewizard;

import java.io.UnsupportedEncodingException;
import java.security.GeneralSecurityException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import android.content.Context;
import android.util.Base64;
import android.util.Log;

public class CMEncrypter {
	private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";
	private static final String KEY_TRANSFORMATION = "AES/ECB/PKCS5Padding";
	private static final String SECRET_KEY_HASH_TRANSFORMATION = "SHA-256";
	private static final String CHARSET = "UTF-8";
	private static final byte[] SALT = new byte[] {47,-69,111,122,-56,68,-10,63,73,84,38,-6,17,-32,58,70,81,-32,71,37};
	
//	private static final String B64PK =
//			"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxhSF+D6AjNJm645fGr4xyTp" +
//			"47TDLzg5bFvdOODhjvEEDN3HEF/nm/qs9qNrrkNXKZfUEXTzc0hucGMXuQa47HqvHcV" +
//			"rSxC57PnOEQ09xlZHN5t2f4QT74olcvxHVrNmAv9YggO/EzYFo/CXnuUAnCXiBtfqc0" +
//			"q9DVs6M0I4TIK3II8S7oyWNcu9Cf6nhWNOTWlE6tg84oEFUPNNbhVsBb94ntocCIgZz" +
//			"abcQU8YHkNEDaFJktFX1G0VEnIhcWpYnV5DiFAsJDcHPCF8I7qLz/fzAWYD8TIWsSpP" +
//			"8BozKdgJQdgcjVRdFSepY72Sh5HuwVrESPQRGZ0IbvWMH33Hp7wIDAQAB";
	
	private static final String B64PK_BETA =
			"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAkt5ZtL3070MiFM6/EboL3pB" +
			"IRXVpC7Khu0Pmzhmq82z4FUU16dEAdCb5PmqM5a3jqVGV6SbyjHB8EVju/vcIwOZ6UT" +
			"IGf5QlOTmPjF1aN7RBRTMTYjbDzafSCYudugLjwncm8Armt/zXh8+Beb/Jesi4kHIBQ" +
			"uB5a/0hlsc2vA6rFD3n1ZiuM6LRNDBPE7aqzNLeBu+3CS0UiQImV1k1NbtjkbqR1iDb" +
			"QyHVt+VG1fkx9hgaNK9PdetkvpJ4F49YVI7txKBAm9yT7cLOD4cuYzMmGWEqOb7mQ+Y" +
			"7fguZZ02ab3aUKktzh9j1MV3ES/Jhask7mi7DQwsdzdCM4L2c2QIDAQAB";
	
	/*
	private static final String B64PK =
			"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAOEAxhSF+D6AjNJm645fGr4xyTp" +
			"47TDLzg5bFvdOODhjvEEDN3HEF/nm/qs9qNrrkNXKZfUEXTzc0hucGMXuQa47HqvHcV" +
			"rSxC57PnOEQ09xlZHN5t2f4QT74olcvxHVrNmAv9YggO/EzYFo/CXnuUAnCxiBtfqc0" +
			"q9DVs6M0I4TIK3II8S7oyWNcu9Cf6nhWNOTWlE6tg84oEFUPNNbhVsBb94ntocCIgZz" +
			"abcQU8YHkNEDeFJktFX1G0VEnIhcWpYnV5DiFAsJDcHPCF8I7qLz/fzAWYD9TIWsSpP" +
			"8BozKdgJQdgcjVRdFSepY72Sh5HuwVrESPQRGZ0IbvWMH33Hp7wIDAQAB";
	
	return B64PK.replace("MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAOEAxhSF+D6AjNJm645fGr4xyTp", "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxhSF+D6AjNJm645fGr4xyTp").
			replace("rSxC57PnOEQ09xlZHN5t2f4QT74olcvxHVrNmAv9YggO/EzYFo/CXnuUAnCxiBtfqc0", "rSxC57PnOEQ09xlZHN5t2f4QT74olcvxHVrNmAv9YggO/EzYFo/CXnuUAnCXiBtfqc0").
			replace("abcQU8YHkNEDeFJktFX1G0VEnIhcWpYnV5DiFAsJDcHPCF8I7qLz/fzAWYD9TIWsSpP", "abcQU8YHkNEDaFJktFX1G0VEnIhcWpYnV5DiFAsJDcHPCF8I7qLz/fzAWYD8TIWsSpP");
	*/
	
	private final Cipher writer;
	private final Cipher reader;
	private final Cipher keyWriter;
	
	public CMEncrypter(Context context, String secureKey) throws CMEncrypterException {
		try {
			this.writer = Cipher.getInstance(TRANSFORMATION);
			this.reader = Cipher.getInstance(TRANSFORMATION);
			this.keyWriter = Cipher.getInstance(KEY_TRANSFORMATION);

			initCiphers( new StringBuilder(secureKey).reverse().toString());
		}
		catch (GeneralSecurityException e) {
			throw new CMEncrypterException(e);
		}
		catch (UnsupportedEncodingException e) {
			throw new CMEncrypterException(e);
		}
	}
	
	public static byte[] getSalt() {
		return SALT;
	}
	
	public static String getPK() {
		return B64PK_BETA; //B64PK;
	}
	
	protected void initCiphers(String secureKey) throws UnsupportedEncodingException, NoSuchAlgorithmException, InvalidKeyException,
		InvalidAlgorithmParameterException {
		IvParameterSpec ivSpec = getIv();
		SecretKeySpec secretKey = getSecretKey(secureKey);
		
		writer.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
		reader.init(Cipher.DECRYPT_MODE, secretKey, ivSpec);
		keyWriter.init(Cipher.ENCRYPT_MODE, secretKey);
	}
	
	protected IvParameterSpec getIv() {
		byte[] iv = new byte[writer.getBlockSize()];
		System.arraycopy("fldsjfodasjifudslfjdsaofshaufihadsf".getBytes(), 0, iv, 0, writer.getBlockSize());
		return new IvParameterSpec(iv);
	}
	
	protected SecretKeySpec getSecretKey(String key) throws UnsupportedEncodingException, NoSuchAlgorithmException {
		byte[] keyBytes = createKeyBytes(key);
		return new SecretKeySpec(keyBytes, TRANSFORMATION);
	}

	protected byte[] createKeyBytes(String key) throws UnsupportedEncodingException, NoSuchAlgorithmException {
		MessageDigest md = MessageDigest.getInstance(SECRET_KEY_HASH_TRANSFORMATION);
		md.reset();
		byte[] keyBytes = md.digest(key.getBytes(CHARSET));
		return keyBytes;
	}
	
	public String encryptValue(String value) {
		if (value == null)
			return null;
		else
			return encrypt(value, writer);
	}
	
	public String decryptValue(String value) {
		if (value == null)
			return null;
		else
			return decrypt(value);
	}

	protected String encrypt(String value, Cipher writer) throws CMEncrypterException {
		byte[] secureValue;
		try {
			secureValue = convert(writer, value.getBytes(CHARSET));
		}
		catch (UnsupportedEncodingException e) {
			Log.i("CMEncrypter::encrypt", e.getMessage());
			throw new CMEncrypterException(e);
		}
		String secureValueEncoded = Base64.encodeToString(secureValue, Base64.NO_WRAP);
		return secureValueEncoded;
	}

	protected String decrypt(String securedEncodedValue) {
		byte[] securedValue = Base64.decode(securedEncodedValue, Base64.NO_WRAP);
		byte[] value = convert(reader, securedValue);
		try {
			return new String(value, CHARSET);
		}
		catch (UnsupportedEncodingException e) {
			Log.i("CMEncrypter::decrypt", e.getMessage());
			throw new CMEncrypterException(e);
		}
	}

	private static byte[] convert(Cipher cipher, byte[] bs) throws CMEncrypterException {
		try {
			return cipher.doFinal(bs);
		}
		catch (Exception e) {
			throw new CMEncrypterException(e);
		}
	}
	
	public static class CMEncrypterException extends RuntimeException {
		private static final long serialVersionUID = 12345L;
		public CMEncrypterException(Throwable e) {
			super(e);
		}
	}
}
