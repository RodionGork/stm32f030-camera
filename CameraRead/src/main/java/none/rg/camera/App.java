package none.rg.camera;

import java.util.*;
import java.awt.*;
import java.awt.image.*;
import javax.swing.*;

public class App {
    
    private Scanner input;
    
    BufferedImage canvas = new BufferedImage(200, 200, BufferedImage.TYPE_INT_RGB);
    
    JFrame frame;
    JPanel panel;
    
    public static void main(String... args) {
        new App().run();
    }
    
    App() {
        for (int i = 0; i < 200 * 200; i++) {
            int v = (int)(Math.random() * 256);
            canvas.setRGB(i % 200, i / 200, v << 8);
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
    
    void run() {
        Scanner scanner = new Scanner(System.in);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            if (line.length() < 3*180*180) {
                continue;
            }
            String[] parts = line.split("\\.");
            System.out.println("line found " + parts.length);
            for (int i = 0; i < parts.length; i++) {
                canvas.setRGB(i % 180, i / 180, Integer.parseInt(parts[i], 16) * 0x10101);
            }
            panel.repaint();
        }
    }
    
}

