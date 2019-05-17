package main.client;

import java.io.*;
import java.net.Socket;
import java.util.Scanner;

public class Client {
  public static void main(String[] args) throws IOException {

    Scanner scanner = new Scanner(System.in);
    Socket socket = null;
    boolean flag = true;

    while (flag) {
      socket = new Socket("127.0.0.1", 20001);
      printMenu();
      int menuChoice = scanner.nextInt();
      if(menuChoice == 1) {
        scanner.nextLine();
        try{
          sendFile(socket, scanner);
        } catch (Exception x) {
          System.out.println("Nie ma takiego pliku.");
        }
      } else if (menuChoice == 0) {
        flag = false;
      } else {
        System.out.println("Nie ma takiej opcji");
      }

      socket.close();
    }

    socket.close();
  }

  private static void sendFile(Socket socket, Scanner scanner) throws IOException {
    System.out.println("Podaj sciezke pliku do przesłania: ");
    String pwd = scanner.nextLine();

    //send file to server
    File file = new File(pwd);

    byte[] bytes = new byte[16 * 1024];
    InputStream in = new FileInputStream(file);
    OutputStream out = socket.getOutputStream();

    int count;
    while ((count = in.read(bytes)) > 0) {
      out.write(bytes, 0, count);
    }

    out.close();
    in.close();
  }


  private static void printMenu() {
    System.out.println("MENU:");
    System.out.println("1. Przeslij plik");
    System.out.println("0. Zakoncz");
  }
}
