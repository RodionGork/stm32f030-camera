package none.rg.camera;

import java.util.*;
import java.awt.*;
import java.awt.image.*;
import javax.swing.*;
import java.io.*;

public class App {
    
    private Scanner input;
    
    BufferedImage canvas = new BufferedImage(400, 300, BufferedImage.TYPE_INT_RGB);
    
    JFrame frame;
    JPanel panel;
    
    public static void main(String... args) throws IOException {
        new App().run();
    }
    
    App() {
        for (int i = 0; i < 400 * 300; i++) {
            int v = (int)(Math.random() * 256);
            canvas.setRGB(i % 400, i / 400, v << 8);
        }
        frame = new JFrame("Camera ov7670");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        panel = new JPanel() {
            public void paintComponent(Graphics g) {
                g.drawImage(canvas, 10, 10, frame);
                System.out.println("paint");
            }
        };
        frame.setContentPane(panel);
        frame.setSize(400, 300);
        frame.setVisible(true);
    }
    
    void run() throws IOException {
        while (true) {
            String line = fetchLine();
            try {
                String[] parts = line.split("\\.");
                System.out.println("line found " + parts.length);
                int w = Integer.parseInt(parts[0]);
                int h = Integer.parseInt(parts[1]);
                int sz = Math.min(w * h, parts.length - 2);
                for (int i = 0; i < sz; i++) {
                    canvas.setRGB(i % w, i / w, Integer.parseInt(parts[i + 2], 16) * 0x10101);
                }
                panel.repaint();
            } catch (Exception e) {
                System.out.println(e);
            }
        }
    }
    
    String fetchLine() throws IOException {
        byte[] buf = new byte[1000000];
        while (System.in.read() != '!');
        int i = 0;
        while (true) {
            int b = System.in.read();
            if (b <= ' ') {
                System.out.println("Ended with " + b);
                break;
            }
            buf[i] = (byte) (0xFF & b);
            i += 1;
        }
        return new String(buf, 0, i);
    }
    
}

