import java.util.concurrent.*;
import java.util.concurrent.locks.*;
import java.util.*;

public class SleepingAssistant {
    private static final int NB_ETUDIANTS = 5;
    private static final int NB_CHAISES = 3;

    private static Semaphore etudiantSem = new Semaphore(0); // Réveille le TA
    private static Semaphore taSem = new Semaphore(0);       // Signale à un étudiant qu’il peut être aidé
    private static Lock lockChaises = new ReentrantLock();

    private static int[] chaises = new int[NB_CHAISES];
    private static int posDebut = 0;
    private static int posFin = 0;
    private static int nbChaisesOccupees = 0;

    static class Etudiant implements Runnable {
        private int id;
        private Random rand = new Random();

        public Etudiant(int id) {
            this.id = id;
        }

        public void run() {
            try {
                while (true) {
                    System.out.println("Étudiant " + id + " programme...");
                    Thread.sleep((rand.nextInt(5) + 1) * 1000); // Simulation programmation

                    lockChaises.lock();
                    try {
                        if (nbChaisesOccupees < NB_CHAISES) {
                            chaises[posFin] = id;
                            posFin = (posFin + 1) % NB_CHAISES;
                            nbChaisesOccupees++;
                            System.out.println("Étudiant " + id + " attend sur une chaise (" +
                                               nbChaisesOccupees + "/" + NB_CHAISES + " occupées).");

                            etudiantSem.release(); // Réveille le TA
                        } else {
                            System.out.println("Étudiant " + id + " ne trouve pas de chaise libre. Il reviendra.");
                            continue; // Recommence à programmer
                        }
                    } finally {
                        lockChaises.unlock();
                    }

                    taSem.acquire(); // Attend que le TA soit prêt à aider
                    System.out.println("Étudiant " + id + " reçoit de l’aide.");
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    static class TA implements Runnable {
        private Random rand = new Random();

        public void run() {
            try {
                while (true) {
                    etudiantSem.acquire(); // Attend qu’un étudiant arrive

                    int idEtudiant = -1;
                    lockChaises.lock();
                    try {
                        idEtudiant = chaises[posDebut];
                        posDebut = (posDebut + 1) % NB_CHAISES;
                        nbChaisesOccupees--;
                        System.out.println("TA appelle l’étudiant " + idEtudiant + " pour de l’aide.");
                    } finally {
                        lockChaises.unlock();
                    }

                    // Simule l’aide
                    System.out.println("TA aide l’étudiant " + idEtudiant + "...");
                    Thread.sleep((rand.nextInt(3) + 1) * 1000);
                    System.out.println("TA a terminé avec l’étudiant " + idEtudiant + ".");

                    taSem.release(); // Laisse l’étudiant continuer
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    public static void main(String[] args) {
        Thread taThread = new Thread(new TA());
        taThread.start();

        for (int i = 1; i <= NB_ETUDIANTS; i++) {
            new Thread(new Etudiant(i)).start();
        }
    }
}
