import tailwindcss from '@tailwindcss/vite';
import { defineConfig } from 'vite';
import { tanstackRouter } from '@tanstack/router-plugin/vite';
import tsconfigPaths from 'vite-tsconfig-paths';
import react from '@vitejs/plugin-react';
export default defineConfig({
    plugins: [
        tailwindcss(),
        tanstackRouter({
            target: 'react',
            autoCodeSplitting: true,
        }),
        react(),
        tsconfigPaths(),
    ],
    build: {
        outDir: 'client',
    },
    envDir: process.env.NODE_ENV === 'production' ? undefined : '../../',
});
